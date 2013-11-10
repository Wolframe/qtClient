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
#include <QWidget>
#include <QMouseEvent>
#include <QApplication>

class WidgetWithDragAndDropBase
{
public:
	static bool handleDragPickEvent( QWidget* this_, QMouseEvent *event, const QVariant& sourceobj);
	static bool handleDragEnterEvent( QWidget* this_, QDragEnterEvent* event);
	static bool handleDropEvent( QWidget* this_, QDropEvent *event, const QVariant& targetobj);
};

template <class WidgetType>
class WidgetWithDragAndDrop :public WidgetType, public WidgetWithDragAndDropBase
{
public:
	WidgetWithDragAndDrop( QWidget* parent = 0)
		:WidgetType(parent)
	{
		WidgetType::setAcceptDrops( true);
	}

	virtual QVariant getDragObjectId( const QPoint&) const
	{
		return QVariant();
	}

	void mousePressEvent( QMouseEvent *event)
	{
		if (event->button() == Qt::LeftButton)
		{
			m_dragStartPosition = event->pos();
		}
		WidgetType::mousePressEvent( event);
	}

	void mouseMoveEvent( QMouseEvent *event)
	{
		if (!(event->buttons() & Qt::LeftButton)) return;
		if ((event->pos() - m_dragStartPosition).manhattanLength() < QApplication::startDragDistance()) return;

		QVariant objid = getDragObjectId( m_dragStartPosition);
		if (!WidgetWithDragAndDropBase::handleDragPickEvent( this, event, objid))
		{
			WidgetType::mouseMoveEvent( event);
		}
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
		QVariant objid = getDragObjectId( event->pos());
		if (!WidgetWithDragAndDropBase::handleDropEvent( this, event, objid))
		{
			WidgetType::dropEvent( event);
		}
	}

private:
	QPoint m_dragStartPosition;
};

#endif

