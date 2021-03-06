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
#include "WidgetVisitor_QListWidget.hpp"
#include "WidgetListener.hpp"
#include "WidgetEnabler.hpp"
#include <QDebug>

WidgetVisitorState_QListWidget::WidgetVisitorState_QListWidget( QWidget* widget_)
	:WidgetVisitorObject(widget_)
	,m_listWidget(qobject_cast<QListWidget*>(widget_))
	,m_mode(Init)
	,m_row(-1)
{}

void WidgetVisitorState_QListWidget::clear()
{
	m_listWidget->clear();
	m_mode = Init;
	m_row = -1;
}

bool WidgetVisitorState_QListWidget::isArrayElement( const QString& name) const
{
	return name == "item";
}

QVariant WidgetVisitorState_QListWidget::valueAt( const QPoint& p) const
{
	QListWidgetItem* itm = m_listWidget->itemAt( p.x(), p.y());
	if (!itm) return QVariant();
	return itm->data( Qt::UserRole);
}

bool WidgetVisitorState_QListWidget::enter( const QString& name, bool writemode)
{
	static const QString item_str( "item");
	if (m_mode == Init && name == item_str)
	{
		if (writemode)
		{
			m_listWidget->addItem( new QListWidgetItem( m_listWidget));
			++m_row;
		}
		else
		{
			if (m_row+1 < m_listWidget->count())
			{
				++m_row;
			}
			else
			{
				return false;
			}
		}
		m_mode = Element;
		return true;
	}
	return false;
}

bool WidgetVisitorState_QListWidget::leave( bool /*writemode*/)
{
	if (m_mode == Element)
	{
		m_mode = Init;
		return true;
	}
	return false;
}


QVariant WidgetVisitorState_QListWidget::property( const QString& name)
{
	switch (m_mode)
	{
		case Element:
			if (name.isEmpty() || name == "text")
			{
				if (m_row >= m_listWidget->count()) return QVariant();
				QListWidgetItem* item = m_listWidget->item( m_row);
				return QVariant( item->text());
			}
			else if (name == "id")
			{
				if (m_row >= m_listWidget->count()) return QVariant();
				QListWidgetItem* item = m_listWidget->item( m_row);
				return item->data( Qt::UserRole);
			}
			else if (name == "tooltip")
			{
				if (m_row >= m_listWidget->count()) return QVariant();
				QListWidgetItem* item = m_listWidget->item( m_row);
				return QVariant( item->toolTip());
			}
			else if (name == "statustip")
			{
				if (m_row >= m_listWidget->count()) return QVariant();
				QListWidgetItem* item = m_listWidget->item( m_row);
				return QVariant( item->statusTip());
			}
			break;

		case Init:
			if (name == "id")
			{
				QList<QVariant> rt;
				int ii=0, nn=m_listWidget->count();
				for (; ii<nn; ++ii)
				{
					QListWidgetItem* item = m_listWidget->item( ii);
					rt.push_back( item->data( Qt::UserRole));
				}
				return rt;
			}
			else if (name == "text")
			{
				QList<QVariant> rt;
				int ii=0, nn=m_listWidget->count();
				for (; ii<nn; ++ii)
				{
					QListWidgetItem* item = m_listWidget->item( ii);
					rt.push_back( QVariant( item->text()));
				}
				return rt;
			}
			else if (name == "selected")
			{
				if (m_listWidget->selectionMode() == QAbstractItemView::SingleSelection)
				{
					foreach( QListWidgetItem *item, m_listWidget->selectedItems())
					{
						return item->data( Qt::UserRole);
					}
					return m_listWidget->property( "_w_selected");
				}
				else
				{
					QList<QVariant> rt;
					foreach( QListWidgetItem *item, m_listWidget->selectedItems())
					{
						rt.append( item->data( Qt::UserRole));
					}
					if (rt.isEmpty())
					{
						return m_listWidget->property( "_w_selected");
					}
					else
					{
						return QVariant( rt);
					}
				}
			}
			else if (name == "unselected")
			{
				QList<QVariant> rt;
				int ri = 0, re = m_listWidget->count();
				for( ; ri < re; ++ri)
				{
					QListWidgetItem *item = m_listWidget->item( ri);
					if (!item->isSelected())
					{
						rt.append( item->data( Qt::UserRole));
					}
				}
				if (rt.isEmpty() && !m_listWidget->count())
				{
					return QVariant();
				}
				else
				{
					return QVariant( rt);
				}
			}
			break;
	}
	return QVariant();
}

bool WidgetVisitorState_QListWidget::setProperty( const QString& name, const QVariant& data)
{
	switch (m_mode)
	{
		case Element:
			if (name.isEmpty() || name == "text")
			{
				if (m_row >= m_listWidget->count()) return false;
				QListWidgetItem* item = m_listWidget->item( m_row);
				item->setText( data.toString());
				return true;
			}
			else if (name == "id")
			{
				if (m_row >= m_listWidget->count()) return false;
				QListWidgetItem* item = m_listWidget->item( m_row);
				item->setData( Qt::UserRole, data);
				return true;
			}
			else if (name == "icon")
			{
				if (m_row >= m_listWidget->count()) return false;
				QListWidgetItem* item = m_listWidget->item( m_row);
				QByteArray decoded = QByteArray::fromBase64( data.toByteArray());
				QPixmap pixmap;
				pixmap.loadFromData( decoded);
				item->setIcon( pixmap);
				return true;
			}
			else if (name == "tooltip")
			{
				if (m_row >= m_listWidget->count()) return false;
				QListWidgetItem* item = m_listWidget->item( m_row);
				item->setToolTip( data.toString());
				return true;
			}
			else if (name == "statustip")
			{
				if (m_row >= m_listWidget->count()) return false;
				QListWidgetItem* item = m_listWidget->item( m_row);
				item->setStatusTip( data.toString());
				return true;
			}
			break;
		case Init:
			if (name == "selected")
			{
				m_listWidget->setProperty( "_w_selected", data);
				initSelected( data);
				return true;
			}
			break;
	}
	return false;
}

void WidgetVisitorState_QListWidget::setState( const QVariant& state)
{
	qDebug() << "set state for list widget" << m_listWidget->objectName();
	for( int i = 0; i < m_listWidget->count( ); i++ )
	{
		QListWidgetItem *item = m_listWidget->item( i );
		item->setSelected( false);
	}
	QList<QVariant> selected = state.toList();
	foreach (const QVariant& elem, selected)
	{
		QList<QListWidgetItem*> itemlist = m_listWidget->findItems( elem.toString(), Qt::MatchExactly);
		QList<QListWidgetItem*>::iterator ti = itemlist.begin(), te = itemlist.end();
		for (; ti != te; ++ti)
		{
			(*ti)->setSelected( true);
		}
	}
}

QVariant WidgetVisitorState_QListWidget::getState() const
{
	QList<QVariant> selected;
	foreach (const QListWidgetItem* item, m_listWidget->selectedItems())
	{
		selected.push_back( QVariant( item->text()));
	}
	return QVariant(selected);
}

void WidgetVisitorState_QListWidget::initSelected( const QVariant& selected)
{
	if (selected.type() == QVariant::List)
	{
		foreach (const QVariant& sel, selected.toList())
		{
			foreach( QListWidgetItem *item, m_listWidget->findItems( sel.toString(), Qt::MatchExactly))
			{
				item->setSelected( true);
			}
		}
	}
	else
	{
		foreach( QListWidgetItem *item, m_listWidget->findItems( selected.toString(), Qt::MatchExactly))
		{
			item->setSelected( true);
		}
	}
}

void WidgetVisitorState_QListWidget::endofDataFeed()
{
	QVariant selected = m_listWidget->property( "_w_selected");
	if (selected.isValid())
	{
		initSelected( selected);
		m_listWidget->setProperty( "_w_selected", QVariant());
	}
}

void WidgetVisitorState_QListWidget::connectDataSignals( WidgetListener::DataSignalType dt, WidgetListener& listener)
{
	switch (dt)
	{
		case WidgetListener::SigChanged:
			QObject::connect( m_listWidget, SIGNAL( currentItemChanged(QListWidgetItem*,QListWidgetItem*)), &listener, SLOT( changed()), Qt::UniqueConnection);
			QObject::connect( m_listWidget, SIGNAL( currentRowChanged(int)), &listener, SLOT( changed()), Qt::UniqueConnection);
			QObject::connect( m_listWidget, SIGNAL( currentTextChanged(const QString&)), &listener, SLOT( changed()), Qt::UniqueConnection);
			QObject::connect( m_listWidget, SIGNAL( itemChanged(QListWidgetItem*)), &listener, SLOT( changed()), Qt::UniqueConnection);
			break;
		case WidgetListener::SigActivated: QObject::connect( m_listWidget, SIGNAL( itemActivated( QListWidgetItem*)), &listener, SLOT( activated()), Qt::UniqueConnection); break;
		case WidgetListener::SigEntered: QObject::connect( m_listWidget, SIGNAL( itemEntered( QListWidgetItem*)), &listener, SLOT( entered()), Qt::UniqueConnection); break;
		case WidgetListener::SigPressed: QObject::connect( m_listWidget, SIGNAL( itemPressed( QListWidgetItem*)), &listener, SLOT( pressed()), Qt::UniqueConnection); break;
		case WidgetListener::SigClicked: QObject::connect( m_listWidget, SIGNAL( itemClicked( QListWidgetItem*)), &listener, SLOT( clicked()), Qt::UniqueConnection); break;
		case WidgetListener::SigDoubleClicked: QObject::connect( m_listWidget, SIGNAL( itemDoubleClicked( QListWidgetItem*)), &listener, SLOT( doubleclicked()), Qt::UniqueConnection);
		case WidgetListener::SigDestroyed: break;
	}
}

void WidgetVisitorState_QListWidget::connectWidgetEnabler( WidgetEnabler& enabler)
{
	QObject::connect( m_listWidget, SIGNAL( currentItemChanged(QListWidgetItem*,QListWidgetItem*)), &enabler, SLOT( changed()), Qt::UniqueConnection);
	QObject::connect( m_listWidget, SIGNAL( currentRowChanged(int)), &enabler, SLOT( changed()), Qt::UniqueConnection);
	QObject::connect( m_listWidget, SIGNAL( currentTextChanged(const QString&)), &enabler, SLOT( changed()), Qt::UniqueConnection);
	QObject::connect( m_listWidget, SIGNAL( itemChanged(QListWidgetItem*)), &enabler, SLOT( changed()), Qt::UniqueConnection);
	QObject::connect( m_listWidget, SIGNAL( itemActivated( QListWidgetItem*)), &enabler, SLOT( changed()), Qt::UniqueConnection);
	QObject::connect( m_listWidget, SIGNAL( itemEntered( QListWidgetItem*)), &enabler, SLOT( changed()), Qt::UniqueConnection);
	QObject::connect( m_listWidget, SIGNAL( itemPressed( QListWidgetItem*)), &enabler, SLOT( changed()), Qt::UniqueConnection);
	QObject::connect( m_listWidget, SIGNAL( itemClicked( QListWidgetItem*)), &enabler, SLOT( changed()), Qt::UniqueConnection);
	QObject::connect( m_listWidget, SIGNAL( itemDoubleClicked( QListWidgetItem*)), &enabler, SLOT( changed()), Qt::UniqueConnection);
}

