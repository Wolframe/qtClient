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
#include "debug/DebugViewWidget.hpp"
#include "debug/DebugLogTree.hpp"
#include <QPushButton>
#include <QHBoxLayout>

DebugViewWidget::DebugViewWidget( QWidget *parent_)
	:QWidget( parent_, Qt::Tool | Qt::WindowTitleHint)
	,m_output(0)
	,m_tree(0)
	,m_layout(0)
{
	m_output = new QTextEdit(this);
	m_output->setReadOnly( true);
	m_tree = new QTreeWidget(this);

	setWindowTitle( tr( "Debug Window"));
	setFixedSize( 640, 480);

	m_layout = new QBoxLayout( QBoxLayout::TopToBottom, this);

	QLabel* label = new QLabel( tr( "Ctrl-Alt-D enables/disables the debug window"), this);
	m_layout->addWidget( label);

	m_layout->addWidget( m_tree);
	m_layout->addWidget( m_output);
	QHBoxLayout* buttons = new QHBoxLayout();
	QPushButton *buttonRefresh = new QPushButton("Refresh");
	QPushButton *buttonClear = new QPushButton("Clear");

	connect( buttonRefresh, SIGNAL(clicked()), this, SLOT(refresh()), Qt::UniqueConnection);
	connect( buttonClear, SIGNAL(clicked()), this, SLOT(clear()), Qt::UniqueConnection);
	
	buttons->addStretch();
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
	refresh();
	show();
	activateWindow();
	m_tree->setFocus();
}

void DebugViewWidget::addSubTree( QTreeWidgetItem* viewnode, const DebugLogTree::NodeStruct* node, int level)
{
	QTreeWidgetItem* item = new QTreeWidgetItem( viewnode);
	item->setData( 0, Qt::UserRole, node->id);
	item->setText( 0, node->name);
	if (level == 0) item->setExpanded( true);

	for (int ii=0; ii<node->chld.size(); ++ii)
	{
		addSubTree( item, node->chld[ ii].data(), level+1);
	}
}

void DebugViewWidget::clear()
{
	clearLogStruct();
	refresh();
}

void DebugViewWidget::refresh()
{
	m_output->clear();
	m_tree->clear();

	DebugLogTree::NodeStructR st = getLogNodeStruct();
	addSubTree( m_tree->invisibleRootItem(), st.data(), 0);
}

void DebugViewWidget::printMessages( QTreeWidgetItem* item, int column)
{
	QString text = getLogMessages( item->data( column, Qt::UserRole).toInt(), m_level);
	m_output->clear();
	m_output->append( text);
}

