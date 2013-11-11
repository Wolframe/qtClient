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
///\brief Interface for drag and drop for widgets
#ifndef _WIDGET_DRAG_AND_DROP_HPP_INCLUDED
#define _WIDGET_DRAG_AND_DROP_HPP_INCLUDED
#include "WidgetId.hpp"
#include "DataLoader.hpp"
#include <QWidget>
#include <QMouseEvent>
#include <QApplication>
#include <QListWidget>
#include <QTableWidget>
#include <QTreeWidget>
#include <QStringList>
#include <QDebug>


class WidgetWithDragAndDropBase
{
public:
	WidgetWithDragAndDropBase( DataLoader* dataLoader_, bool debug_)
		:m_dataLoader(dataLoader_),m_debug(debug_){}

protected:
	void sendDropRequest( QWidget* dropWidget, const WidgetId& dragWidgetid, const QString& action, const QVariant& dropvalue);

public:
	bool handleDragPickEvent( QWidget* this_, QMouseEvent *event);
	bool handleDragEnterEvent( QWidget* this_, QDragEnterEvent* event);
	bool handleDropEvent( QWidget* this_, QDropEvent *event);

private:
	DataLoader* m_dataLoader;
	bool m_debug;
};


template <class WidgetType>
class WidgetWithDragAndDrop :public WidgetType, public WidgetWithDragAndDropBase
{
public:
	WidgetWithDragAndDrop( DataLoader* dataLoader_, QWidget* parent_, const QString& name_, bool debug_)
		:WidgetType(parent_)
		,WidgetWithDragAndDropBase(dataLoader_,debug_)
	{
		WidgetType::setObjectName( name_);
		WidgetType::setAcceptDrops( true);
	}

	void mousePressEvent( QMouseEvent *event)
	{
		if (event->button() == Qt::LeftButton)
		{
			m_dragStartPosition = event->pos();
			m_drag = true;
		}
		WidgetType::mousePressEvent( event);
	}

	void mouseMoveEvent( QMouseEvent *event)
	{
		if (!m_drag) return;
		if (!(event->buttons() & Qt::LeftButton)) return;
		if ((event->pos() - m_dragStartPosition).manhattanLength() < QApplication::startDragDistance()) return;

		if (!WidgetWithDragAndDropBase::handleDragPickEvent( this, event))
		{
			WidgetType::mouseMoveEvent( event);
		}
		m_drag = false;
	}

	void dragEnterEvent( QDragEnterEvent *event)
	{
		if (!WidgetWithDragAndDropBase::handleDragEnterEvent( this, event))
		{
			WidgetType::dragEnterEvent( event);
		}
	}

	void dragLeaveEvent( QDragLeaveEvent * event)
	{
		WidgetType::dragLeaveEvent( event);
	}

	void dropEvent( QDropEvent *event)
	{
		if (!WidgetWithDragAndDropBase::handleDropEvent( this, event))
		{
			WidgetType::dropEvent( event);
		}
	}

	QStringList mimeTypes() const
	{
		QStringList rt;
		rt.append( WIDGETID_MIMETYPE);
		return rt;
	}

	Qt::DropActions supportedDropActions() const
	{
		return Qt::CopyAction | Qt::MoveAction;
	}

private:
	QPoint m_dragStartPosition;
	bool m_drag;
};

#endif

