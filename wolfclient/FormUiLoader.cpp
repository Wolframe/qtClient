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
///\brief Own UI Loader implementation to add features to custom widgets
#include "FormUiLoader.hpp"
#include "WidgetDragAndDrop.hpp"
#include <QTreeWidget>
#include <QLineEdit>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QTableWidget>
#include <QTextEdit>

QWidget* FormUiLoader::createWidget( const QString& className, QWidget* parent, const QString& name)
{
	if (className == "QTreeWidget")
	{
		QTreeWidget* rt = new WidgetWithDragAndDrop<QTreeWidget>( m_dataLoader, parent, name, m_debug);
		rt->setDragEnabled( true);
		rt->showDropIndicator();
		return rt;
	}
	else if (className == "QListWidget")
	{
		QListWidget* rt = new WidgetWithDragAndDrop<QListWidget>( m_dataLoader, parent, name, m_debug);
		rt->setDragEnabled( true);
		rt->showDropIndicator();
		return rt;
	}
	else if (className == "QTableWidget")
	{
		QTableWidget* rt = new WidgetWithDragAndDrop<QTableWidget>( m_dataLoader, parent, name, m_debug);
		rt->setDragEnabled( true);
		return rt;
	}
	else
	{
		return QUiLoader::createWidget( className, parent, name);
	}
}

