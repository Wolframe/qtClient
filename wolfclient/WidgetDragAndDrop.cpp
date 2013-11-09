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

static bool containsPoint( const QWidget* actWidget, const QVariant& cond)
{
	QRect globalWidgetRect = QRect(actWidget->mapToGlobal(QPoint(0,0)), actWidget->size());
	QPoint mousePos = cond.toPoint();
	if (actWidget->objectName().isEmpty() || actWidget->objectName().startsWith("qt_")) return false;
#ifdef WOLFRAME_LOWLEVEL_DEBUG
	qDebug() << "[drag/drop handler] test position" << mousePos << "in" << globalWidgetRect << actWidget->objectName();
#endif
	return (globalWidgetRect.contains( mousePos));
}

bool mousePressEventHandleDrag( QWidget* mainwidget, QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton && event->type() == QEvent::MouseButtonPress)
	{
		WidgetVisitor visitor( mainwidget);
		QPoint eventPos = event->globalPos();
#ifdef WOLFRAME_LOWLEVEL_DEBUG
		const char* eventnam = "unknown";
		if (event->button() == Qt::LeftButton) eventnam = "LeftButton";
		else if (event->button() == Qt::RightButton) eventnam = "RightButton";
		QRect globalWidgetRect = QRect( mainwidget->mapToGlobal( QPoint(0,0)), mainwidget->size());

		qDebug() << "[drag/drop handler] mousePressEvent" << eventnam << "at" << eventPos << "in" << globalWidgetRect;
#endif
		foreach (QWidget* subnode, visitor.findSubNodes( containsPoint, eventPos))
		{
			WidgetVisitor subvisitor( subnode);
			if (subvisitor.hasDrag())
			{
				qDebug() << "[drag/drop handler] has drag" << subnode->objectName() << eventPos;

				QDrag *drag = new QDrag( subnode);
				QMimeData *mimeData = new QMimeData;
				mimeData->setText( subvisitor.widgetid());
				//Data( WIDGETID_MIMETYPE, subvisitor.widgetid().toLatin1());
				drag->setMimeData( mimeData);
				drag->setPixmap( QPixmap( ":/images/16x16/copy.png"));
				event->accept();
		
				Qt::DropAction dropAction = drag->exec();
				switch (dropAction)
				{
					case Qt::CopyAction: qDebug() << "Handle event copy action drag on widget" << subvisitor.widgetid(); break;
					case Qt::MoveAction: qDebug() << "Handle event move action drag on widget" << subvisitor.widgetid(); break;
					case Qt::IgnoreAction: qDebug() << "Ignore drag event on widget" << subvisitor.widgetid(); continue;
					default: qCritical() << "internal: illegal state in handle drag event"; continue;
				}
				return true;
			}
		}
	}
	return false;
}

bool mousePressEventHandleDragEnter( QWidget* mainwidget, QDragEnterEvent *event)
{
	const QMimeData* mimeData = event->mimeData();
	if (mimeData->hasFormat( WIDGETID_MIMETYPE))
	{
		WidgetVisitor visitor( mainwidget);
		QPoint eventPos = event->pos() + mainwidget->mapToGlobal( QPoint(0,0));

#ifdef WOLFRAME_LOWLEVEL_DEBUG
		QRect globalWidgetRect = QRect( mainwidget->mapToGlobal( QPoint(0,0)), mainwidget->size());

		qDebug() << "[drag/drop handler] dragEnterEvent at" << eventPos << "in" << globalWidgetRect;
#endif
		foreach (QWidget* subnode, visitor.findSubNodes( containsPoint, eventPos))
		{
#ifdef WOLFRAME_LOWLEVEL_DEBUG
			qDebug() << "[drag/drop handler] check drag enter" << subnode->objectName() << eventPos;
#endif
			WidgetVisitor subvisitor( subnode);
			Qt::DropAction dropAction = event->dropAction();
			const char* dropActionStr = 0;
			switch (dropAction)
			{
				case Qt::CopyAction: dropActionStr = "copy"; break;
				case Qt::IgnoreAction: 
				case Qt::MoveAction: dropActionStr = "move"; break;
				default: continue;
			}
			if (dropActionStr)
			{
				WidgetId widgetId( subnode);
				QString propname = QString("drop") + dropActionStr + ":" + widgetId.objectName();

				if (subnode->property(propname.toLatin1()).isValid())
				{
					event->acceptProposedAction();
					qDebug() << "[drag/drop handler] drag enter accept " << subnode->objectName() << "at" << eventPos;
					return true;
				}
			}
		}
	}
	return false;
}

bool mousePressEventHandleDrop( QWidget* mainwidget, QDropEvent *event)
{
	WidgetVisitor visitor( mainwidget);
	QPoint eventPos = event->pos() + mainwidget->mapToGlobal( QPoint(0,0));

#ifdef WOLFRAME_LOWLEVEL_DEBUG
	qDebug() << "[drag/drop handler] drop at" << eventPos;
#endif
	foreach (QWidget* subnode, visitor.findSubNodes( containsPoint, eventPos))
	{
#ifdef WOLFRAME_LOWLEVEL_DEBUG
		qDebug() << "[drag/drop handler] check drop" << subnode->objectName() << eventPos;
#endif
		WidgetVisitor subvisitor( subnode);
		Qt::DropAction dropAction = event->dropAction();
		const char* dropActionStr = 0;
		switch (dropAction)
		{
			case Qt::CopyAction: dropActionStr = "copy"; break;
			case Qt::IgnoreAction: 
			case Qt::MoveAction: dropActionStr = "move"; break;
			default: continue;
		}
		if (dropActionStr)
		{
			WidgetId widgetId( subnode);
			QString propname = QString("drop") + dropActionStr + ":" + widgetId.objectName();

			QVariant action = subnode->property(propname.toLatin1());
			if (action.isValid())
			{
				qDebug() << "[drag/drop handler] define 'drag' link" << widgetId.toString();
				subvisitor.defineLink( "drag", widgetId.toString());

				// ... submit request
				qDebug() << "[drag/drop handler] submit drop request" << action.toString();
			}
		}
	}
	return false;
}

