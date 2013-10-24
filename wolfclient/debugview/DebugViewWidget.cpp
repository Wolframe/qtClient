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
#include "debugview/DebugViewWidget.hpp"
#include "debugview/DebugLogTree.hpp"
#include <QPushButton>
#include <QHBoxLayout>
#include <QHeaderView>

DebugViewWidget::DebugViewWidget( QWidget *parent_)
	:QWidget( parent_, Qt::Tool | Qt::WindowTitleHint)
	,m_icon_error( QString( ":/images/debug-error.png"))
	,m_icon_warning( QString( ":/images/debug-warning.png"))
	,m_icon_ok( QString( ":/images/debug-ok.png"))
	,m_msglist(0)
	,m_tree(0)
	,m_layout(0)
	,m_levelSelect(0)
	,m_level(LogDebug)
{
	m_msglist = new QTableWidget( this);
	m_msglist->setColumnCount( 2);
	m_msglist->setHorizontalHeaderLabels( QStringList() << "Type" << "Message");

	m_tree = new QTreeWidget( this);
	m_tree->setHeaderLabel( "Navigation");

	setWindowTitle( tr( "Debug Window"));
	resize( 640, 480);

	m_layout = new QBoxLayout( QBoxLayout::TopToBottom, this);

	QLabel* label = new QLabel( tr( "Ctrl-Alt-D enables/disables the debug window"), this);
	m_layout->addWidget( label);

	m_layout->addWidget( m_tree);
	m_layout->addWidget( m_msglist);
	QHBoxLayout* buttons = new QHBoxLayout();
	QPushButton *buttonRefresh = new QPushButton("Refresh");
	QPushButton *buttonClear = new QPushButton("Clear");
	m_levelSelect = new QComboBox();
	m_levelSelect->addItem( "Debug", (int)LogDebug);
	m_levelSelect->addItem( "Warning", (int)LogWarning);
	m_levelSelect->addItem( "Error", (int)LogCritical);
	m_levelSelect->setCurrentIndex( 0);

	connect( buttonRefresh, SIGNAL(clicked()), this, SLOT(refresh()), Qt::UniqueConnection);
	connect( buttonClear, SIGNAL(clicked()), this, SLOT(clear()), Qt::UniqueConnection);
	connect( m_levelSelect, SIGNAL(currentIndexChanged(int)), this, SLOT(debugLevelChanged(int)), Qt::UniqueConnection);
	
	buttons->addStretch();
	buttons->addWidget( m_levelSelect);
	buttons->addWidget( buttonRefresh);
	buttons->addWidget( buttonClear);
	m_layout->addLayout( buttons);
	m_tree->setFocus();

	QObject::connect( m_tree, SIGNAL( itemClicked( QTreeWidgetItem*,int)), this, SLOT( printMessages( QTreeWidgetItem*,int)), Qt::UniqueConnection);
}

DebugViewWidget::~DebugViewWidget()
{
	if (m_layout) delete m_layout;
}

void DebugViewWidget::bringToFront()
{
	show();
	activateWindow();
	m_tree->setFocus();
	refresh();
}

static QString extractInternalMarker( const QString& name)
{
	QString rt;
	QString::const_iterator si = name.begin(), se = name.end();
	for (; si != se; ++si)
	{
		if (*si == ':' || *si == '#')
		{
			for (; (si+1) != se && *(si+1) >= '0' && *(si+1) <= '9'; ++si);
			continue;
		}
		if (*si == '.')
		{
			rt.push_back( ' ');
		}
		else
		{
			rt.push_back( *si);
		}
	}
	return rt;
}

void DebugViewWidget::addSubTree( QTreeWidgetItem* viewnode, const DebugLogTree::NodeStruct* node, int level)
{
	QTreeWidgetItem* item = new QTreeWidgetItem( viewnode);
	item->setData( 0, Qt::UserRole, node->id);
	if (level == 0 && node->name.isEmpty())
	{
		item->setText( 0, "All");
		item->setToolTip( 0, "Show all messages (also the unclassified)");
		//... root node name
	}
	else
	{
		item->setText( 0, extractInternalMarker( node->name));
		item->setToolTip( 0, node->name);
	}
	switch (node->maxlevel)
	{
		case LogDebug: item->setIcon( 0, m_icon_ok); break;
		case LogWarning: item->setIcon( 0, m_icon_warning); break;
		case LogCritical: item->setIcon( 0, m_icon_error); break;
		case LogFatal: item->setIcon( 0, m_icon_error); break;
	}
	if (level == 0) item->setExpanded( true);
	item->setSelected( false);

	for (int ii=0; ii<node->chld.size(); ++ii)
	{
		addSubTree( item, node->chld[ ii].data(), level+1);
	}
}

void DebugViewWidget::clear()
{
	clearLogStruct();
	clearMessages();
	m_tree->clear();
	refresh();
}

void DebugViewWidget::refresh()
{
	QList<QTreeWidgetItem*> sel = m_tree->selectedItems();
	int selid = sel.isEmpty()?-1:sel.at(0)->data( 0, Qt::UserRole).toInt();
	m_tree->clear();

	DebugLogTree::NodeStructR st = getLogNodeStruct();
	addSubTree( m_tree->invisibleRootItem(), st.data(), 0);
	if (selid >= 0)
	{
		printMessages( selid);
	}
	else
	{
		clearMessages();
	}
}

void DebugViewWidget::clearMessages()
{
	m_msglist->setSortingEnabled( false );
	m_msglist->clearContents();
	for (int ii=m_msglist->rowCount()-1; ii >= 0; ii--)
	{
		m_msglist->removeRow( ii);
	}
}

void DebugViewWidget::printMessages( int nodeid)
{
	clearMessages();
	QList<DebugLogTree::MessageStruct> msgar = getLogMessages( nodeid, m_level);
	QList<DebugLogTree::MessageStruct>::const_iterator mi = msgar.begin(), me = msgar.end();
	for (int rowidx=0; mi != me; ++mi,++rowidx)
	{
		m_msglist->insertRow( rowidx);
		QTableWidgetItem* msgitem = new QTableWidgetItem( mi->text);
		msgitem->setFlags( msgitem->flags() ^ Qt::ItemIsEditable);
		msgitem->setToolTip( mi->text);
		QTableWidgetItem* lvitem = 0;
		m_msglist->setItem( rowidx, 1, msgitem);
		switch (mi->level)
		{
			case LogDebug:
				lvitem = new QTableWidgetItem( "Debug");
				break;
			case LogWarning:
				lvitem = new QTableWidgetItem( m_icon_warning, "Warning");
				break;
			case LogCritical:
				lvitem = new QTableWidgetItem( m_icon_error, "Critical");
				break;
			case LogFatal:
				lvitem = new QTableWidgetItem( m_icon_error, "Fatal");
				break;
		}
		if (lvitem)
		{
			lvitem->setFlags( lvitem->flags() ^ Qt::ItemIsEditable);
			m_msglist->setItem( rowidx, 0, lvitem);
		}
	}
	for (int ii = 0; ii < m_msglist->columnCount(); ii++)
	{
		m_msglist->resizeColumnToContents( ii);
	}
	for (int ii = 0; ii < m_msglist->rowCount(); ii++) {
		m_msglist->resizeRowToContents( ii);
	}
	m_msglist->horizontalHeader()->setStretchLastSection( true);
	m_msglist->setShowGrid( false);
}

void DebugViewWidget::printMessages( QTreeWidgetItem* item, int column)
{
	int nodeid = item->data( column, Qt::UserRole).toInt();
	printMessages( nodeid);
}

void DebugViewWidget::debugLevelChanged( int idx)
{
	QVariant lv = m_levelSelect->itemData( idx);
	if (lv.isValid())
	{
		m_level = (LogLevel)lv.toInt();
	}
	else
	{
		m_level = LogDebug;
	}
	refresh();
}

