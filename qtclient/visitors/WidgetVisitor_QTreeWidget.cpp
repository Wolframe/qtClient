/************************************************************************

 Copyright (C) 2011 - 2013 Project Wolframe.
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
#include "WidgetVisitor_QTreeWidget.hpp"
#include "WidgetListener.hpp"
#include "WidgetEnabler.hpp"
#include <QDebug>

WidgetVisitorState_QTreeWidget::WidgetVisitorState_QTreeWidget( QWidget* widget_)
	:WidgetVisitorObject(widget_)
	,m_treeWidget(qobject_cast<QTreeWidget*>(widget_))
	,m_elementname(widget_->objectName())
	,m_mode(Init)
{
	QTreeWidgetItem* header = m_treeWidget->headerItem();
	for( int ii = 0; ii < header->columnCount(); ii++) {
		QString headerText = header->data( ii, Qt::DisplayRole).toByteArray();
		m_headers << headerText;
	}
	if (m_elementname.isEmpty())
	{
		qCritical() << "empty or illegal object name defined for tree widget";
	}
	m_stk.push_back( StackElement( m_treeWidget->invisibleRootItem()));
}

void WidgetVisitorState_QTreeWidget::clear()
{
	m_treeWidget->clear();
	m_mode = Init;
	m_stk.clear();
	m_stk.push_back( StackElement( m_treeWidget->invisibleRootItem()));
}

bool WidgetVisitorState_QTreeWidget::enter( const QString& name, bool writemode)
{
	static const QString item_str( "item");
	static const QString row_str( "row");
	if (m_mode != List && name == item_str)
	{
		if (writemode)
		{
			m_stk.push_back( StackElement( new QTreeWidgetItem( m_stk.top().item)));
		}
		else
		{
			if (m_stk.top().readpos >= m_stk.top().item->childCount()) return false;
			m_stk.push_back( m_stk.top().item->child( m_stk.top().readpos++));
		}
		m_mode = Tree;
		return true;
	}
	else if (m_mode != Tree && name == row_str)
	{
		if (m_stk.size() != 1) return false;
		if (writemode)
		{
			m_stk.push_back( StackElement( new QTreeWidgetItem( m_stk.top().item)));
		}
		else
		{
			if (m_stk.top().readpos >= m_stk.top().item->childCount()) return false;
			m_stk.push_back( m_stk.top().item->child( m_stk.top().readpos++));
		}
		m_mode = List;
		return true;
	}
	return false;
}

bool WidgetVisitorState_QTreeWidget::leave( bool /*writemode*/)
{
	if (m_stk.isEmpty()) return false;
	m_stk.pop_back();
	return true;
}

bool WidgetVisitorState_QTreeWidget::isArrayElement( const QString& name)
{
	static const QString item_str( "item");
	static const QString row_str( "row");
	if (m_mode != List && name == item_str) return true;
	if (m_mode != Tree && name == row_str) return true;
	return false;
}

QVariant WidgetVisitorState_QTreeWidget::property( const QString& name)
{
	static const QString selected_str( "selected");
	static const QString unselected_str( "unselected");
	static const QString id_str( "id");
	static const QString tooltip_str( "tooltip");
	if (name == selected_str)
	{
		if (m_treeWidget->selectionMode() == QAbstractItemView::SingleSelection)
		{
			foreach (const QTreeWidgetItem* sel, m_treeWidget->selectedItems())
			{
				return sel->data( 0, Qt::UserRole);
			}
			return m_treeWidget->property( "_w_selected");
		}
		else
		{
			QList<QVariant> idlist;
			foreach (const QTreeWidgetItem* sel, m_treeWidget->selectedItems())
			{
				idlist.push_back( sel->data( 0, Qt::UserRole));
			}
			if (idlist.isEmpty())
			{
				return m_treeWidget->property( "_w_selected");
			}
			return QVariant( idlist);
		}
// ABa: This needs discussion and a better model (and syntax)
	} else if( name.startsWith( "selected_" ) ) {
		QList<QString> parts = name.split( '_' );
		if( m_treeWidget->selectionMode( ) == QAbstractItemView::SingleSelection ) {
			int col = m_headers.indexOf( parts[1] );
			foreach( const QTreeWidgetItem* sel, m_treeWidget->selectedItems( ) ) {
				if( col != -1 ) {
					return sel->data( col, Qt::DisplayRole );
				} else if( parts[1] == "id" ) {
					return sel->data( 0, Qt::UserRole );
				}
			}
		} else {
			qCritical( ) << "Multiple selection subproperties not supported";
		}
	}
	else if (name == unselected_str)
	{
		QList<QVariant> idlist;
		QTreeWidgetItemIterator it( m_treeWidget);
		while (*it)
		{
			if (!(*it)->isSelected())
			{
				idlist.push_back( (*it)->data( 0, Qt::UserRole));
			}
			++it;
		}
		return QVariant( idlist);
	}
	if (m_stk.isEmpty()) return QVariant();
	int col = m_headers.indexOf( name);
	if (col != -1)
	{
		return QVariant( m_stk.top().item->text( col));
	}
	if (name == id_str)
	{
		return m_stk.top().item->data( 0, Qt::UserRole);
	}
	if (name == tooltip_str)
	{
		return m_stk.top().item->toolTip(0);
	}
	return QVariant();
}

bool WidgetVisitorState_QTreeWidget::setProperty( const QString& name, const QVariant& data)
{
	static const QString id_str( "id");
	static const QString selected_str( "selected");
	static const QString tooltip_str( "tooltip");
	static const QString icon_str( "icon");
	if (m_stk.isEmpty()) return false;
	int col = m_headers.indexOf( name);
	if (col != -1)
	{
		m_stk.top().item->setText( col, data.toString());
		return true;
	}
	if (name == id_str)
	{
		m_stk.top().item->setData( 0, Qt::UserRole, data);
		return true;
	}
	else if (name == tooltip_str)
	{
		m_stk.top().item->setToolTip( 0,data.toString());
		return true;
	}
	else if (name == selected_str)
	{
		m_treeWidget->setProperty( "_w_selected", data);
		initSelected( data);
		return true;
	}
	else if (name == icon_str)
	{
		qDebug() << "ICON:" << data.toString() << col;
		if (data.toString().startsWith( ":"))
		{
// internal resource
			m_stk.top().item->setIcon( 0, QIcon( data.toString()));
		}
		else
		{
// base64 encoded data from server
			QByteArray decoded = QByteArray::fromBase64( data.toByteArray());
			QPixmap pixmap;
			pixmap.loadFromData( decoded);
			m_stk.top().item->setIcon( 0, pixmap);
		}
		return true;
	}
	return false;
}

enum StateTag {None,Open,Close,Expand,Select,ExpandSelect};
struct DebugFunction
{
	static const char* stateTagName( StateTag i)
	{
		static const char* ar[] = {"None","Open","Close","Expand","Select","ExpandSelect"};
		return ar[i];
	}
};
static const char g_tagchr[] = "_ocES*";

static QVariant getStateElementKey( StateTag tag, const QVariant& elem)
{
	QString rt;
	rt.push_back( (QChar)g_tagchr[ (int)tag]);
	rt.append( elem.toString());
	return QVariant(rt);
}

static QString stateElementValue( const QVariant& elemkey)
{
	QString kk = elemkey.toString();
	return kk.mid( 1, kk.size()-1);
}

static StateTag stateElementTag( const QVariant& elemkey)
{
	QString kk = elemkey.toString();
	const char* pos = strchr( g_tagchr, kk.at( 0).toLatin1());
	return pos?(StateTag)(pos-g_tagchr):None;
}

static void skipTag( QList<QVariant>::const_iterator& itr, const QList<QVariant>::const_iterator end)
{
	++itr;
	for (int taglevel=1; itr!=end && taglevel > 0; ++itr)
	{
		switch (stateElementTag(*itr))
		{
			case Open: ++taglevel; break;
			case Close: --taglevel; break;
			case None:
			case Expand:
			case Select:
			case ExpandSelect: break;
		}
	}
	--itr;
}

static QTreeWidgetItem* findchild( const QTreeWidgetItem* item, int keyidx, const QVariant& elemkey)
{
	int ii=0,chldcnt = item->childCount();
	QString elemkeystr = elemkey.toString();
	for (; ii<chldcnt; ++ii)
	{
		QTreeWidgetItem* chld = item->child( ii);
		if (elemkeystr == chld->data( keyidx, Qt::UserRole).toString()) return chld;
	}
	return 0;
}

static QTreeWidgetItem* findsubnode( const QTreeWidgetItem* item, int keyidx, const QVariant& elemkey)
{
	int ii=0,chldcnt = item->childCount();
	QString elemkeystr = elemkey.toString();
	for (; ii<chldcnt; ++ii)
	{
		QTreeWidgetItem* chld = item->child( ii);
		if (elemkeystr == chld->data( keyidx, Qt::UserRole).toString()) return chld;
		chld = findsubnode( chld, keyidx, elemkey);
		if (chld) return chld;
	}
	return 0;
}

static void setElementState( QList<QVariant>::const_iterator& itr, const QList<QVariant>::const_iterator& end, int keyidx, QTreeWidgetItem* item)
{
	QTreeWidgetItem* chld;
	while (itr != end)
	{
		StateTag st = stateElementTag(*itr);
		switch (st)
		{
			case Open:
				chld = findchild( item, keyidx, stateElementValue( *itr));
				if (!chld)
				{
					skipTag( itr, end);
				}
				else
				{
					++itr;
					setElementState( itr, end, keyidx, chld);
				}
				break;
			case Close:
				++itr;
				return;
			case None:
				++itr;
				break;
			case Expand:
				chld = findchild( item, keyidx, stateElementValue( *itr));
				if (chld) chld->setExpanded( true);
				++itr;
				break;
			case Select:
				chld = findchild( item, keyidx, stateElementValue( *itr));
				if (chld) chld->setSelected( true);
				++itr;
				break;
			case ExpandSelect:
				chld = findchild( item, keyidx, stateElementValue( *itr));
				if (chld)
				{
					chld->setExpanded( true);
					chld->setSelected( true);
				}
				++itr;
				break;
			default:
				++itr;
		}
	}
}

void WidgetVisitorState_QTreeWidget::setState( const QVariant& state)
{
	qDebug() << "set state for tree widget" << m_treeWidget->objectName();
	static const QString id_str( "id");
	QStack<StackElement> stk;
	int keyidx = m_headers.indexOf( id_str);
	if (keyidx < 0) keyidx = 0; //... first element is key if "id" not defined

	if (state.isValid())
	{
		QList<QVariant> statelist = state.toList();
		QList<QVariant>::const_iterator itr = statelist.begin();
		setElementState( itr, statelist.end(), keyidx, m_treeWidget->invisibleRootItem());

		if (m_mode == List)
		{
			for( int ii = 0; ii < m_headers.size(); ii++)
			{
				m_treeWidget->resizeColumnToContents( ii);
			}
		}
	}
}

static StateTag itemStateTag( const QTreeWidgetItem* item)
{
	StateTag rt = None;
	if (item->isExpanded())
	{
		rt = item->isSelected()?ExpandSelect:Expand;
	}
	else if (item->isSelected())
	{
		rt = Select;
	}
	return rt;
}

static void getElementState( QList<QVariant>& rt, int keyidx, QTreeWidgetItem* root)
{
	int readpos = 0;
	for (; readpos<root->childCount(); ++readpos)
	{
		QTreeWidgetItem* item = root->child( readpos);
		if (item->isExpanded() || item->isSelected())
		{
			rt.push_back( getStateElementKey( itemStateTag( item), item->data( keyidx, Qt::UserRole)));
		}
		int oldsize = rt.size();
		rt.push_back( getStateElementKey( Open, item->data( keyidx, Qt::UserRole)));
		getElementState( rt, keyidx, item);
		if (oldsize+1 == rt.size())
		{
			rt.pop_back();
		}
		else
		{
			rt.push_back( getStateElementKey( Close, QVariant()));
		}
	}
}

QVariant WidgetVisitorState_QTreeWidget::getState() const
{
	QList<QVariant> rt;
	static const QString id_str( "id");
	int keyidx = m_headers.indexOf( id_str);
	if (keyidx < 0) keyidx = 0; //... first element is key if "id" not defined

	getElementState( rt, keyidx, m_treeWidget->invisibleRootItem());
	return QVariant(rt);
}

void WidgetVisitorState_QTreeWidget::initSelected( const QVariant& selected)
{
	static const QString id_str( "id");
	QStack<StackElement> stk;
	int keyidx = m_headers.indexOf( id_str);
	if (keyidx < 0) keyidx = 0; //... first element is key if "id" not defined
	QTreeWidgetItem* root = m_treeWidget->invisibleRootItem();

	if (m_treeWidget->selectionMode() == QAbstractItemView::SingleSelection)
	{
		m_treeWidget->clearSelection( );
	}

	if (selected.type() == QVariant::List)
	{		
		QList<QVariant> selectedlist = selected.toList();
		foreach (const QVariant& sel, selectedlist)
		{
			QTreeWidgetItem* item = findsubnode( root, keyidx, sel);
			if (item)
			{
				for (QTreeWidgetItem* prn = item->parent(); prn && prn != root; prn = prn->parent())
				{
					prn->setExpanded( true);
				}
				item->setSelected( true);
			}
		}
	}
	else
	{
		QTreeWidgetItem* item = findsubnode( root, keyidx, selected);
		if (item)
		{
			for (QTreeWidgetItem* prn = item->parent(); prn && prn != root; prn = prn->parent())
			{
				prn->setExpanded( true);
			}
			item->setSelected( true);
		}
	}
}

void WidgetVisitorState_QTreeWidget::endofDataFeed()
{
	QVariant selected = m_treeWidget->property( "_w_selected");
	if (selected.isValid())
	{
		initSelected( selected);
		m_treeWidget->setProperty( "_w_selected", QVariant());
	}
}

void WidgetVisitorState_QTreeWidget::connectDataSignals( WidgetListener::DataSignalType dt, WidgetListener& listener)
{
	switch (dt)
	{
		case WidgetListener::SigChanged:
			QObject::connect( m_treeWidget, SIGNAL( currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), &listener, SLOT( changed()), Qt::UniqueConnection);
			QObject::connect( m_treeWidget, SIGNAL( itemChanged(QTreeWidgetItem*,int)), &listener, SLOT( changed()), Qt::UniqueConnection);
			break;
		case WidgetListener::SigActivated: QObject::connect( m_treeWidget, SIGNAL( itemActivated( QTreeWidgetItem*,int)), &listener, SLOT( activated()), Qt::UniqueConnection); break;
		case WidgetListener::SigEntered: QObject::connect( m_treeWidget, SIGNAL( itemEntered( QTreeWidgetItem*,int)), &listener, SLOT( entered()), Qt::UniqueConnection); break;
		case WidgetListener::SigPressed: QObject::connect( m_treeWidget, SIGNAL( itemPressed( QTreeWidgetItem*,int)), &listener, SLOT( pressed()), Qt::UniqueConnection); break;
		case WidgetListener::SigClicked: QObject::connect( m_treeWidget, SIGNAL( itemClicked( QTreeWidgetItem*,int)), &listener, SLOT( clicked()), Qt::UniqueConnection); break;
		case WidgetListener::SigDoubleClicked: QObject::connect( m_treeWidget, SIGNAL( itemDoubleClicked( QTreeWidgetItem*,int)), &listener, SLOT( doubleclicked()), Qt::UniqueConnection); break;
		case WidgetListener::SigDestroyed: break;
	}
}

void WidgetVisitorState_QTreeWidget::connectWidgetEnabler( WidgetEnabler& enabler)
{
	QObject::connect( m_treeWidget, SIGNAL( currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), &enabler, SLOT( changed()), Qt::UniqueConnection);
	QObject::connect( m_treeWidget, SIGNAL( itemChanged(QTreeWidgetItem*,int)), &enabler, SLOT( changed()), Qt::UniqueConnection);
	QObject::connect( m_treeWidget, SIGNAL( itemActivated( QTreeWidgetItem*,int)), &enabler, SLOT( changed()), Qt::UniqueConnection);
	QObject::connect( m_treeWidget, SIGNAL( itemEntered( QTreeWidgetItem*,int)), &enabler, SLOT( changed()), Qt::UniqueConnection);
	QObject::connect( m_treeWidget, SIGNAL( itemPressed( QTreeWidgetItem*,int)), &enabler, SLOT( changed()), Qt::UniqueConnection);
	QObject::connect( m_treeWidget, SIGNAL( itemClicked( QTreeWidgetItem*,int)), &enabler, SLOT( changed()), Qt::UniqueConnection);
	QObject::connect( m_treeWidget, SIGNAL( itemDoubleClicked( QTreeWidgetItem*,int)), &enabler, SLOT( changed()), Qt::UniqueConnection);
}


