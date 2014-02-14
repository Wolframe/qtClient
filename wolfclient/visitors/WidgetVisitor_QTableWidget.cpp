/************************************************************************

 Copyright (C) 2011 - 2014 Project Wolframe.
 All rights reserved.

 This file is part of Project Wolframe.

 Commercial Usage
    Licensees holding valid Project Wolframe Commercial licenses may
    use this file in accordance with the Project Wolframe
    Commercial License Agreement provided with the Software or,
    alternatively, in accordance with the terms contained
    in a written agreement between the licensee and Project Wolframe.

 GNU General Public License Usage
    Alternatively, you can redistribute this file and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Wolframe is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Wolframe.  If not, see <http://www.gnu.org/licenses/>.

 If you have questions regarding the use of this file, please contact
 Project Wolframe.

************************************************************************/
#include "WidgetVisitor_QTableWidget.hpp"
#include "WidgetListener.hpp"
#include "WidgetListener.hpp"
#include "WidgetEnabler.hpp"
#include <QDebug>
#include <QLabel>
#include <QBuffer>
#include <QHeaderView>
#include <QSignalMapper>
#include <QWidget>

int WidgetVisitorState_QTableWidget::findRowHeader( const QString& name) const
{
	int re = m_rowcount;
	for (int ri=0; ri<re; ++ri)
	{
		QTableWidgetItem* item = m_tableWidget->verticalHeaderItem( ri);
		if (item && item->text() == name) return ri;
	}
	return -1;
}

int WidgetVisitorState_QTableWidget::findColHeader( const QString& name) const
{
	int ce = m_columncount;
	for (int ci=0; ci<ce; ++ci)
	{
		QTableWidgetItem* item = m_tableWidget->horizontalHeaderItem( ci);
		if (item && item->text() == name) return ci;
	}
	return -1;
}

WidgetVisitorState_QTableWidget::WidgetVisitorState_QTableWidget( QWidget* widget_)
	:WidgetVisitorObject(widget_)
	,m_tableWidget(qobject_cast<QTableWidget*>(widget_))
	,m_mode(Init)
	,m_row(-1)
	,m_column(-1)
	,m_rowcount(0)
	,m_columncount(0)
	,m_valueidx(-1)
{
	while (m_tableWidget->verticalHeaderItem( m_rowcount)) ++m_rowcount;
	while (m_tableWidget->horizontalHeaderItem( m_columncount)) ++m_columncount;
	QVariant vc = m_tableWidget->property( "_w_valueidx");
	if (vc.isValid())
	{
		m_valueidx = vc.toInt();
		if (m_rowcount) m_rowcount++;
		else if (m_columncount) m_columncount++;
	}
	else
	{
		if (m_rowcount)
		{
			m_valueidx = m_rowcount++;
			m_tableWidget->setSortingEnabled( false); //... see comment in leave(..)
			m_tableWidget->insertRow( m_valueidx);
			m_tableWidget->setRowHidden( m_valueidx, true);
			m_tableWidget->setSortingEnabled( true);
			m_tableWidget->setProperty( "_w_valueidx", m_valueidx);
		}
		else if (m_columncount)
		{
			m_valueidx = m_columncount++;
			m_tableWidget->setSortingEnabled( false); //... see comment in leave(..)
			m_tableWidget->insertColumn( m_valueidx);
			m_tableWidget->setColumnHidden( m_valueidx, true);
			m_tableWidget->setSortingEnabled( true);
			m_tableWidget->setProperty( "_w_valueidx", m_valueidx);
		}
	}
}

WidgetVisitorState_QTableWidget::~WidgetVisitorState_QTableWidget()
{
	foreach (QWidget* wdg, m_cellwidgets) if (wdg) delete wdg;
}

QVariant WidgetVisitorState_QTableWidget::valueAt( const QPoint& p) const
{
	QTableWidgetItem* itm = m_tableWidget->itemAt( p.x(), p.y());
	if (!itm) return QVariant();
	if (m_rowcount)
	{
		return getColumnValue( itm->column());
	}
	else if (m_columncount)
	{
		return getRowValue( itm->row());
	}
	else
	{
		return itm->data( Qt::UserRole);
	}
}

void WidgetVisitorState_QTableWidget::clear()
{
	m_tableWidget->setSortingEnabled( false );
	m_tableWidget->clearContents();
	for (int i = m_tableWidget->rowCount()-1; i >= 0; i--)
	{
		m_tableWidget->removeRow( i);
	}
	m_mode = Init;
	m_row = -1;
	m_column = -1;
	m_tableWidget->setSortingEnabled( true );
}

bool WidgetVisitorState_QTableWidget::enter( const QString& name, bool writemode)
{
	switch (m_mode)
	{
		case Init:
		{
			if (name == "row")
			{
				if (writemode)
				{
					m_row = m_tableWidget->rowCount(); //... default insertion at bottom of table
				}
				else
				{
					if (++m_row >= m_tableWidget->rowCount()) return false;
				}
				m_mode = Row;
				return true;
			}
			if (name == "column")
			{
				if (writemode)
				{
					m_column = m_tableWidget->columnCount(); //... default insertion at end of table
				}
				else
				{
					if (++m_column >= m_tableWidget->columnCount()) return false;
				}
				m_mode = Column;
				return true;
			}
			return false;
		}
		case Row:
		{
			int ci = findColHeader( name);
			if (ci == -1) return false;
			m_column = ci;
			if (writemode)
			{
				while (m_column >= m_items.size()) m_items.push_back( QVariant());
				while (m_column >= m_cellwidgets.size()) m_cellwidgets.push_back( 0);
			}
			m_mode = RowData;
			return true;
		}
		case Column:
		{
			int ci = findRowHeader( name);
			if (ci == -1) return false;
			m_row = ci;
			if (writemode)
			{
				while (m_row >= m_items.size()) m_items.push_back( QVariant());
				while (m_row >= m_cellwidgets.size()) m_cellwidgets.push_back( 0);
			}
			m_mode = ColumnData;
			return true;
		}
		case RowData:
		case ColumnData:
			return false;
	}
	return false;
}

void WidgetVisitorState_QTableWidget::fill_cell( int row, int col, int itemidx)
{
	QTableWidgetItem* item = m_tableWidget->item( row, col);
	if (!item)
	{
		item = new QTableWidgetItem( m_items.at( itemidx).toString());
		item->setFlags( item->flags() ^ Qt::ItemIsEditable);
		if (m_items.at( itemidx).isValid())
		{
			item->setData( Qt::UserRole, m_items.at( itemidx));
		}
		m_tableWidget->setItem( row, col, item);
	}
	if (m_cellwidgets.at( itemidx))
	{
		m_tableWidget->setCellWidget( row, col, m_cellwidgets.at( itemidx));
		m_cellwidgets[ itemidx] = 0;
	}
}

bool WidgetVisitorState_QTableWidget::leave( bool writemode)
{
	switch (m_mode)
	{
		case Init:
			return false;
		case Row:
			if (writemode)
			{
				m_tableWidget->setSortingEnabled( false);
				//... [NOTE:qtcentre.org] Disable sorting because,
				// with sorting enabled, as soon as you insert a new item 
				// in a row, QTableWidget runs the sorting check on the 
				// new row item and relocates the item as per set sorting 
				// order, this means that the item row number is changed,
				// and the row variable which you hold is no more valid.
				if (m_row == m_tableWidget->rowCount())
				{
					m_tableWidget->insertRow( m_row);
				}
				for (int col=0; col<m_items.size(); ++col)
				{
					fill_cell( m_row, col, col);
				}
				m_items.clear();
				m_cellwidgets.clear();
				m_tableWidget->setSortingEnabled( true);
			}
			m_row = -1;
			m_column = -1;
			m_mode = Init;
			return true;
		case Column:
			if (writemode)
			{
				m_tableWidget->setSortingEnabled( false);
				if (m_column == m_tableWidget->columnCount())
				{
					m_tableWidget->insertColumn( m_column);
				}
				for (int row=0; row<m_items.size(); ++row)
				{
					fill_cell( row, m_column, row);
				}
				m_items.clear();
				m_cellwidgets.clear();
				m_tableWidget->setSortingEnabled( true);
			}
			m_row = -1;
			m_column = -1;
			m_mode = Init;
			return true;
		case RowData:
			m_mode = Row;
			return true;
		case ColumnData:
			m_mode = Column;
			return true;
	}
	return false;
}

bool WidgetVisitorState_QTableWidget::isArrayElement( const QString& name) const
{
	if (m_mode == Init && (name == "row" || name == "column")) return true;
	return false;
}

void WidgetVisitorState_QTableWidget::set_pixmap( int /*row*/, int itemidx, const QVariant& data)
{
	QByteArray decoded = QByteArray::fromBase64( data.toByteArray());
	QPixmap pixmap;
	pixmap.loadFromData( decoded);
	QLabel *label = new QLabel();
	label->setPixmap( pixmap);
	label->setFixedSize( pixmap.size());
	if (m_cellwidgets.at( itemidx)) delete m_cellwidgets[ itemidx];
	m_cellwidgets[ itemidx] = label;
}

QVariant WidgetVisitorState_QTableWidget::get_pixmap( int row, int col) const
{
	const QWidget* cellwidget = m_tableWidget->cellWidget( row, col);
	if (!cellwidget) return QVariant();
	const QLabel* label = qobject_cast<const QLabel*>( cellwidget);
	if (!label) return QVariant();
	const QPixmap* pixmap = label->pixmap();
	if (!pixmap) return QVariant();
	QByteArray bytes;
	QBuffer buffer(&bytes);
	buffer.open( QIODevice::WriteOnly);
	pixmap->save( &buffer);
	return QVariant( bytes.toBase64());
}

QVariant WidgetVisitorState_QTableWidget::getDataValue( const char* propertyname, int idx) const
{
	QVariant values_p = m_tableWidget->property( propertyname);
	QList<QVariant> values = values_p.toList();
	if (idx < 0 || idx >= values.size()) return QVariant();
	return values.at( idx);
}

void WidgetVisitorState_QTableWidget::setDataValue( const char* propertyname, int idx, QVariant value)
{
	QVariant values_p = m_tableWidget->property( propertyname);
	QList<QVariant> values = values_p.toList();
	while (idx > values.size()) values.push_back( QVariant());
	if (idx == values.size())
	{
		values.push_back( value);
	}
	else
	{
		values[ idx] = value;
	}
	m_tableWidget->setProperty( propertyname, values);
}

QVariant WidgetVisitorState_QTableWidget::getRowValue( int row) const
{
	int ci = findColHeader( "id");
	if (ci < 0)
	{
		ci = (m_columncount?m_valueidx:0);
	}
	QTableWidgetItem* item = m_tableWidget->item( row, ci);
	if (!item)
	{
		qCritical() << "failed to get table value on row" << row << "column" << ci;
		return QVariant();
	}
	else
	{
		return item->data( Qt::UserRole);
	}
}

void WidgetVisitorState_QTableWidget::setRowValue( int row, QVariant value)
{
	int ci = findColHeader( "id");
	if (ci < 0)
	{
		ci = (m_columncount?m_valueidx:0);
	}
	QTableWidgetItem* item = m_tableWidget->item( row, ci);
	if (!item)
	{
		while (ci >= m_items.size()) m_items.push_back( QVariant());
		m_items[ ci] = value;
		while (ci >= m_cellwidgets.size()) m_cellwidgets.push_back( 0);
	}
	else
	{
		item->setData( Qt::UserRole, value);
	}
}

QVariant WidgetVisitorState_QTableWidget::getColumnValue( int col) const
{
	int ri = findRowHeader( "id");
	if (ri < 0)
	{
		ri = (m_rowcount?m_valueidx:0);
	}
	QTableWidgetItem* item = m_tableWidget->item( ri, col);
	if (!item)
	{
		qCritical() << "failed to get table value on column" << col << "row" << ri;
		return QVariant();
	}
	else
	{
		return item->data( Qt::UserRole);
	}
}

void WidgetVisitorState_QTableWidget::setColumnValue( int col, QVariant value)
{
	int ri = findRowHeader( "id");
	if (ri < 0)
	{
		ri = (m_rowcount?m_valueidx:0);
	}
	QTableWidgetItem* item = m_tableWidget->item( ri, col);
	if (!item)
	{
		while (ri >= m_items.size()) m_items.push_back( QVariant());
		m_items[ ri] = value;
	}
	else
	{
		item->setData( Qt::UserRole, value);
	}
}

int WidgetVisitorState_QTableWidget::findSelectedRow( QVariant value)
{
	int ci = findColHeader( "id");
	if (ci < 0)
	{
		ci = (m_columncount?m_valueidx:0);
	}
	int ri=0, re=m_tableWidget->rowCount();
	for (; ri < re; ++ri)
	{
		QTableWidgetItem* item = m_tableWidget->item( ri, ci);
		if (item && item->data( Qt::UserRole) == value) return ri;
	}
	return -1;
}

int WidgetVisitorState_QTableWidget::findSelectedColumn( QVariant value)
{
	int ri = findRowHeader( "id");
	if (ri < 0)
	{
		ri = (m_rowcount?m_valueidx:0);
	}
	int ci=0, ce=m_tableWidget->columnCount();
	for (; ci < ce; ++ci)
	{
		QTableWidgetItem* item = m_tableWidget->item( ri, ci);
		if (item && item->data( Qt::UserRole) == value) return ci;
	}
	return -1;
}

QVariant WidgetVisitorState_QTableWidget::getSelectedValue() const
{
	QVariant val = m_tableWidget->property( "_w_selected");
	if (val.isValid()) return val;
	if (m_tableWidget->selectionMode() == QAbstractItemView::NoSelection)
	{
		return QVariant();
	}
	QList<QTableWidgetItem*> sel_items = m_tableWidget->selectedItems();
	int idx = 0;
	if (m_rowcount)
	{
		int ri = findRowHeader( "id");
		if (ri >= 0)
		{
			idx = ri;
		}
		else
		{
			idx = m_valueidx;
		}
	}
	else if (m_columncount)
	{
		int ci = findColHeader( "id");
		if (ci >= 0)
		{
			idx = ci;
		}
		else
		{
			idx = m_valueidx;
		}
	}
	if (m_tableWidget->selectionMode() == QAbstractItemView::SingleSelection)
	{
		if (sel_items.isEmpty()) return QVariant();
		QTableWidgetItem* item = sel_items.at(0);
		if (!item) return QVariant();
		QVariant rt;
		if (m_rowcount)
		{
			rt = getColumnValue( item->column());
		}
		else if (m_columncount)
		{
			rt = getRowValue( item->row());
		}
		else
		{
			rt = item->data( Qt::UserRole);
		}
		return rt;
	}
	else
	{
		QList<QVariant> rt;
		foreach (QTableWidgetItem* item, sel_items)
		{
			if (item)
			{
				if (m_rowcount)
				{
					if (idx == item->row())
					{
						rt.push_back( item->data( Qt::UserRole));
					}
				}
				else if (m_columncount)
				{
					if (idx == item->column())
					{
						rt.push_back( item->data( Qt::UserRole));
					}
				}
				else
				{
					rt.push_back( item->data( Qt::UserRole));
				}
			}
			else
			{
				rt.push_back( QVariant());
			}
		}
		return rt.isEmpty()?QVariant():rt;
	}
}


QVariant WidgetVisitorState_QTableWidget::property( const QString& name)
{
	QTableWidgetItem* item;
	switch (m_mode)
	{
		case Init:
			if (name == "selected")
			{
				QVariant sel = getSelectedValue();
				return sel;
			}
			break;
		case Row:
			if (name == "title")
			{
				item = m_tableWidget->verticalHeaderItem( m_row);
				if (item)
				{
					return QVariant( item->text().toLatin1());
				}
			}
			else if (name == "id")
			{
				return getRowValue( m_row);
			}
			break;
		case Column:
			if (name == "title")
			{
				item = m_tableWidget->horizontalHeaderItem( m_column);
				if (item)
				{
					return QVariant( item->text().toLatin1());
				}
			}
			else if (name == "id")
			{
				return getColumnValue( m_column);
			}
			break;
		case RowData:
		case ColumnData:
			if (name.isEmpty() || name == "text")
			{
				item = m_tableWidget->item( m_row, m_column);
				if (item) return item->data( Qt::UserRole);
			}
			else if (name == "tooltip")
			{
				item = m_tableWidget->item( m_row, m_column);
				return QVariant( item->toolTip());
			}
			else if (name == "pixmap")
			{
				return get_pixmap( m_row, m_column);
			}
	}
	return QVariant();
}

bool WidgetVisitorState_QTableWidget::setProperty( const QString& name, const QVariant& data)
{
	QHash<QString,int>::const_iterator itr;
	QTableWidgetItem* item;
	switch (m_mode)
	{
		case Init:
			if (name == "selected")
			{
				m_tableWidget->setProperty( "_w_selected", data);
				initSelected( data);
				return true;
			}
			break;
		case Row:
			if (name == "title")
			{
				int ci = findRowHeader( data.toString());
				if (m_row >= 0 || ci < 0) return false;
				m_row = ci;
				return true;
			}
			else if (name == "id")
			{
				setRowValue( m_row, data);
				return true;
			}
			break;
		case Column:
			if (name == "title")
			{
				int ci = findColHeader( data.toString());
				if (m_column >= 0 || ci < 0) return false;
				m_column = ci;
				return true;
			}
			else if (name == "id")
			{
				setColumnValue( m_column, data);
				return true;
			}
			break;
		case RowData:
			if (name.isEmpty())
			{
				m_items[ m_column] = data;
				return true;
			}
			if (name == "pixmap")
			{
				set_pixmap( m_row, m_column, data);
				return true;
			}
			if (name == "tooltip")
			{
				item = m_tableWidget->item( m_row, m_column);
				item->setToolTip( data.toString());
				return true;
			}
			break;
		case ColumnData:
			if (name.isEmpty())
			{
				m_items[ m_row] = data;
				return true;
			}
			if (name == "pixmap")
			{
				set_pixmap( m_row, m_row, data);
				return true;
			}
			if (name == "tooltip")
			{
				item = m_tableWidget->item( m_row, m_column);
				item->setToolTip( data.toString());
				return true;
			}
			break;
	}
	return false;
}

void WidgetVisitorState_QTableWidget::setState( const QVariant& state)
{
	qDebug() << "set state for table" << m_tableWidget->objectName();
	if (state.type() == QVariant::List)
	{
		if (state.toList().size() >= 1)
		{
			initSelected( state.toList().at(0));
		}
	}
	else if (state.isValid())
	{
		initSelected( state);
	}
	for( int ii = 0; ii < m_tableWidget->columnCount(); ii++)
	{
		m_tableWidget->resizeColumnToContents( ii);
	}
	
	// make the data fit in height unless we specified a fixed height
	// as dynamic property
	QString heightPropName = QString( "row:height" );
	QVariant heightPropValue = m_tableWidget->property( heightPropName.toLatin1( ) );
	unsigned int fixedHeight = heightPropValue.toUInt( );
	for( int ii = 0; ii < m_tableWidget->rowCount(); ii++)
	{
		if( fixedHeight > 0 ) {
			m_tableWidget->setRowHeight( ii, fixedHeight );
		} else {
			m_tableWidget->resizeRowToContents( ii);
		}
	}
	
	// apply explicit sizes here (from dynamic properties), found no other
	// way to do this (there is only horizontalHeaderDefaultSectionSize)
	for( int i = 0; i < m_tableWidget->columnCount( ); i++ ) {
		QString propName = QString( "column:%1:width" ).arg( i );
		QVariant propValue = m_tableWidget->property( propName.toLatin1( ) );
		if( propValue.isValid( ) && propValue.toUInt( ) > 0 ) {
			m_tableWidget->setColumnWidth( i, propValue.toUInt( ) );
		}		
	}

// make sure we scroll to the last element of the selection
	foreach( QTableWidgetItem *item, m_tableWidget->selectedItems( ) ) {
		m_tableWidget->scrollToItem( item );
	}
}

QVariant WidgetVisitorState_QTableWidget::getState() const
{
	QList<QVariant> rt;
	QVariant selected = getSelectedValue();
	rt.push_back( selected);
	return QVariant(rt);
}

void WidgetVisitorState_QTableWidget::initSelected( const QVariant& selected)
{
	QTableWidgetSelectionRange all( 0, 0, m_tableWidget->rowCount(), m_tableWidget->columnCount());
	m_tableWidget->setRangeSelected( all, false);
	if (selected.type() == QVariant::List)
	{
		foreach (QVariant elem, selected.toList())
		{
			initSelected( elem);
		}
	}
	if (m_columncount)
	{
		int row = findSelectedRow( selected);
		int col = findColHeader( "id");
		if (col < 0)
		{
			col = 0;
		}
		if (row >= 0)
		{
			QTableWidgetSelectionRange line( row, 0, row, m_columncount-1);
			m_tableWidget->setRangeSelected( line, true);
		}
		return;
	}
	if (m_rowcount)
	{
		int col = findSelectedColumn( selected);
		int row = findRowHeader( "id");
		if (row < 0)
		{
			row = 0;
		}
		if (col >= 0)
		{
			QTableWidgetSelectionRange line( 0, col, m_rowcount-1, col);
			m_tableWidget->setRangeSelected( line, true);
		}
		return;
	}
	
	qCritical() << "cannot find selected line in table:" << selected;
}

void WidgetVisitorState_QTableWidget::endofDataFeed()
{
	QVariant selected = m_tableWidget->property( "_w_selected");
	if (selected.isValid())
	{
		initSelected( selected);
		m_tableWidget->setProperty( "_w_selected", QVariant());
	}
}

void WidgetVisitorState_QTableWidget::connectDataSignals( WidgetListener::DataSignalType dt, WidgetListener& listener)
{
	switch (dt)
	{
		case WidgetListener::SigChanged: QObject::connect( m_tableWidget, SIGNAL( cellChanged( int,int)), &listener, SLOT( changed()), Qt::UniqueConnection); break;
		case WidgetListener::SigActivated: QObject::connect( m_tableWidget, SIGNAL( cellActivated( int,int)), &listener, SLOT( activated()), Qt::UniqueConnection); break;
		case WidgetListener::SigEntered: QObject::connect( m_tableWidget, SIGNAL( cellEntered( int,int)), &listener, SLOT( entered()), Qt::UniqueConnection); break;
		case WidgetListener::SigPressed: QObject::connect( m_tableWidget, SIGNAL( cellPressed( int,int)), &listener, SLOT( pressed()), Qt::UniqueConnection); break;
		case WidgetListener::SigClicked: QObject::connect( m_tableWidget, SIGNAL( cellClicked( int,int)), &listener, SLOT( clicked()), Qt::UniqueConnection); break;
		case WidgetListener::SigDoubleClicked: QObject::connect( m_tableWidget, SIGNAL( cellDoubleClicked( int,int)), &listener, SLOT( doubleclicked()), Qt::UniqueConnection);
		case WidgetListener::SigDestroyed: break;
	}
}

void WidgetVisitorState_QTableWidget::connectWidgetEnabler( WidgetEnabler& enabler)
{
	QObject::connect( m_tableWidget, SIGNAL( cellChanged( int,int)), &enabler, SLOT( changed()), Qt::UniqueConnection);
	QObject::connect( m_tableWidget, SIGNAL( cellActivated( int,int)), &enabler, SLOT( changed()), Qt::UniqueConnection);
	QObject::connect( m_tableWidget, SIGNAL( cellEntered( int,int)), &enabler, SLOT( changed()), Qt::UniqueConnection);
	QObject::connect( m_tableWidget, SIGNAL( cellPressed( int,int)), &enabler, SLOT( changed()), Qt::UniqueConnection);
	QObject::connect( m_tableWidget, SIGNAL( cellClicked( int,int)), &enabler, SLOT( changed()), Qt::UniqueConnection);
	QObject::connect( m_tableWidget, SIGNAL( cellDoubleClicked( int,int)), &enabler, SLOT( changed()), Qt::UniqueConnection);
}

