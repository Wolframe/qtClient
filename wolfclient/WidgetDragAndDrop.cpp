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
///\brief Implement drag and drop for widgets
#include "WidgetVisitor.hpp"
#include "WidgetDragAndDrop.hpp"
#include "WidgetId.hpp"
#include <QDrag>
#include <QMimeData>

#define WOLFRAME_LOWLEVEL_DEBUG

bool WidgetWithDragAndDropBase::handleDragPickEvent( QWidget* this_, QMouseEvent *event, const QVariant& /*sourceobj*/)
{
#ifdef WOLFRAME_LOWLEVEL_DEBUG
	qDebug() << "[drag/drop handler] handle drag pick" << this_->objectName();
#endif
	WidgetVisitor visitor( this_);

	QDrag *drag = new QDrag( this_);
	QMimeData *mimeData = new QMimeData;
	mimeData->setData( WIDGETID_MIMETYPE, visitor.widgetid().toLatin1());
	drag->setMimeData( mimeData);
	drag->setPixmap( QPixmap( ":/images/16x16/copy.png"));

	Qt::DropAction dropAction = drag->exec( Qt::CopyAction | Qt::MoveAction);
	switch (dropAction)
	{
		case Qt::CopyAction: qDebug() << "Handle event copy action drag on widget" << visitor.widgetid(); break;
		case Qt::MoveAction: qDebug() << "Handle event move action drag on widget" << visitor.widgetid(); break;
		case Qt::IgnoreAction: qDebug() << "Ignore drag event on widget" << visitor.widgetid(); return false;
		default: qCritical() << "internal: illegal state in handle drag event"; return false;
	}
	event->accept();
	return true;
}

bool WidgetWithDragAndDropBase::handleDragEnterEvent( QWidget* this_, QDragEnterEvent* event)
{
#ifdef WOLFRAME_LOWLEVEL_DEBUG
	qDebug() << "[drag/drop handler] handle drag enter" << this_->objectName();
#endif
	WidgetVisitor visitor( this_);
	Qt::DropAction dropAction = event->dropAction();
	const char* dropActionStr = 0;
	switch (dropAction)
	{
		case Qt::CopyAction: dropActionStr = "copy"; break;
		case Qt::MoveAction: dropActionStr = "move"; break;
		case Qt::IgnoreAction: break;
		default: break;
	}
	if (dropActionStr)
	{
		WidgetId widgetId( this_);
		QString propname = QString("drop") + dropActionStr + ":" + widgetId.objectName();

		if (this_->property(propname.toLatin1()).isValid())
		{
			event->acceptProposedAction();
			qDebug() << "[drag/drop handler] drag enter accept " << this_->objectName() << "at" << event->pos();
			return true;
		}
	}
	return false;
}

bool WidgetWithDragAndDropBase::handleDropEvent( QWidget* this_, QDropEvent *event, const QVariant& /*targetobj*/)
{
#ifdef WOLFRAME_LOWLEVEL_DEBUG
	qDebug() << "[drag/drop handler] handle drop" << this_->objectName();
#endif
	WidgetVisitor visitor( this_);
	Qt::DropAction dropAction = event->dropAction();
	const char* dropActionStr = 0;
	switch (dropAction)
	{
		case Qt::CopyAction: dropActionStr = "copy"; break;
		case Qt::MoveAction: dropActionStr = "move"; break;
		case Qt::IgnoreAction: return false;
		default: return false;
	}
	if (dropActionStr)
	{
		WidgetId widgetId( this_);
		QString propname = QString("drop") + dropActionStr + ":" + widgetId.objectName();

		QVariant action = this_->property(propname.toLatin1());
		if (action.isValid())
		{
			qDebug() << "[drag/drop handler] define 'drag' link" << widgetId.toString();
			visitor.defineLink( "drag", widgetId.toString());

			// ... submit request
			qDebug() << "[drag/drop handler] submit drop request" << action.toString();
		}
	}
	return false;
}

