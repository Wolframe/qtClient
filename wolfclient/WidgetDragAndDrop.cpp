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
///\brief Implement drag and drop for widgets
#include "WidgetDragAndDrop.hpp"
#include "WidgetId.hpp"
#include "WidgetRequest.hpp"
#include "WidgetDataSignal.hpp"
#include "FormWidget.hpp"
#include "debugview/DebugLogTree.hpp"
#include <QDrag>
#include <QMimeData>

#define WOLFRAME_LOWLEVEL_DEBUG

static bool hasWidgetDropDefined( const QWidget* widget)
{
	foreach (const QByteArray& prop, widget->dynamicPropertyNames())
	{
		if (prop.startsWith("dropmove"))
		{
			if (prop.startsWith("dropmove:") || prop == "dropmove")
			{
				qDebug() << "widget" << widget->objectName() << "will accept drops of visitor objects (widgetid mime type)";
				return true;
			}
		}
		else if (prop.startsWith("dropcopy"))
		{
			if (prop.startsWith("dropcopy:") || prop == "dropcopy")
			{
				qDebug() << "widget" << widget->objectName() << "will accept drops of visitor objects (widgetid mime type)";
				return true;
			}
		}
	}
	return false;
}

void enableDropForFormWidgetChildren( QWidget* widget)
{
	if (hasWidgetDropDefined( widget)) widget->setAcceptDrops( true);
	foreach (QWidget* chld, widget->findChildren<QWidget*>())
	{
		if (hasWidgetDropDefined( chld)) chld->setAcceptDrops( true);
	}
}

static const char* dropActionName( const Qt::DropAction& dropAction)
{
	switch (dropAction)
	{
		case Qt::CopyAction: return "copy";
		case Qt::MoveAction: return "move";
		default: return 0;
	}
}

void WidgetWithDragAndDropBase::handleDatasignal( WidgetVisitor& visitor, const char* signame)
{
	DataSignalHandler dshandler( m_dataLoader, m_debug);
	QList<DataSignalReceiver> rcvlist;
	QWidget* widget = visitor.widget();
	QVariant prop = widget->property( QByteArray("datasignal:") + signame);
	if (prop.isValid())
	{
		foreach (const QString& rcvaddr, parseDataSignalList( prop.toString()))
		{
			rcvlist.append( getDataSignalReceivers( visitor, rcvaddr, true));
		}
		foreach (const DataSignalReceiver& receiver, rcvlist)
		{
			dshandler.trigger( receiver.first, receiver.second);
			qDebug() << "[drag/drop handler] refresh of" << getWidgetId( receiver.second) << "invoked by datasignal" << signame;
		}
	}
	else
	{
		dshandler.trigger( signame, widget);
	}
}

bool WidgetWithDragAndDropBase::handleDragPickEvent( QWidget* this_, QMouseEvent *event)
{
	QDrag *drag;
	WidgetVisitor visitor( this_);
	FormWidget* form = visitor.formwidget();
	if (!form) return false;

	openLogStruct( form->logId());
	openLogStruct( "drag");
	
	qDebug() << "[drag/drop handler] handle drag pick" << visitor.widgetid() << "item" << visitor.valueAt( event->pos()) << (this_->acceptDrops()?"accepting drops":"not accepting drops");
	event->accept();
	drag = new QDrag( this_);
	QMimeData *mimeData = new QMimeData;
	mimeData->setData( WIDGETID_MIMETYPE, visitor.widgetid().toLatin1());
	drag->setMimeData( mimeData);	
	// drag->setPixmap( QPixmap( ":/images/16x16/copy.png"));
	closeLogStruct(2);
	Qt::DropAction dropAction = drag->exec( Qt::CopyAction | Qt::MoveAction);
	//... drag->exec is blocking till end of drop. so the following code is executed after !

	const char* dropActionStr = dropActionName(dropAction);
	openLogStruct( visitor.formwidget()->logId());
	openLogStruct( "drag");

	qDebug() << "[drag/drop handler] 'drop' action result (" << (dropActionStr?dropActionStr:"ignored") << ")" << visitor.widgetid();
	if (dropAction == Qt::MoveAction)
	{
		// Refresh of involved widget via a datasignal
		handleDatasignal( visitor, "drag");
	}
	closeLogStruct(2);
	return true;
}

bool WidgetWithDragAndDropBase::handleDragMoveEvent( QWidget*, QDragMoveEvent* event)
{
#ifdef WOLFRAME_LOWLEVEL_DEBUG
	qDebug() << "[drag/drop handler] move to" << event->pos();
#endif
	if (event->mimeData()->hasFormat( WIDGETID_MIMETYPE))
	{
		event->acceptProposedAction();
		return true;
	}
	return false;
}

bool WidgetWithDragAndDropBase::handleDragEnterEvent( QWidget* this_, QDragEnterEvent* event)
{
	const char* acceptType = 0;
	WidgetVisitor visitor( this_);
	FormWidget* form = visitor.formwidget();
	if (!form) return false;

	openLogStruct( form->logId());
	openLogStruct( "drag");

	if (event->mimeData()->hasFormat( WIDGETID_MIMETYPE))
	{
		WidgetId widgetId( this_);

		if (widgetId.objectName() == this_->objectName())
		{
			if (this_->property("dropmove").isValid())
			{
				if (event->dropAction() == Qt::IgnoreAction)
				{
					acceptType = "accept forced";
					event->setDropAction( Qt::MoveAction);
					event->accept();
				}
				else if (event->dropAction() == Qt::MoveAction)
				{
					acceptType = "accept proposed";
					event->acceptProposedAction();
				}
			}
			if (!acceptType && this_->property("dropcopy").isValid())
			{
				if (event->dropAction() == Qt::IgnoreAction)
				{
					acceptType = "accept forced";
					event->setDropAction( Qt::CopyAction);
					event->accept();
				}
				else if (event->dropAction() == Qt::CopyAction)
				{
					acceptType = "accept proposed";
					event->acceptProposedAction();
				}
			}
		}
		else
		{
			QString propname_move = QString("dropmove:") + widgetId.objectName();
			QString propname_copy = QString("dropcopy:") + widgetId.objectName();
	
			if (this_->property(propname_move.toLatin1()).isValid())
			{
				if (event->dropAction() == Qt::IgnoreAction)
				{
					acceptType = "accept forced";
					event->setDropAction( Qt::MoveAction);
					event->accept();
				}
				else if (event->dropAction() == Qt::MoveAction)
				{
					acceptType = "accept proposed";
					event->acceptProposedAction();
				}
			}
			if (!acceptType && this_->property(propname_copy.toLatin1()).isValid())
			{
				if (event->dropAction() == Qt::IgnoreAction)
				{
					acceptType = "accept forced";
					event->setDropAction( Qt::CopyAction);
					event->accept();
				}
				else if (event->dropAction() == Qt::CopyAction)
				{
					acceptType = "accept proposed";
					event->acceptProposedAction();
				}
			}
		}
		qDebug() << "[drag/drop handler]" << (acceptType?acceptType:"reject") << "drag enter for mime type" << WIDGETID_MIMETYPE << "on" << visitor.widgetid() << "at" << event->pos() << "action" << dropActionName( event->dropAction());
	}
	else
	{
		qDebug() << "[drag/drop handler] drag enter in" << visitor.widgetid() << "with unknown mime type" << event->mimeData()->formats();
	}
	if (acceptType) event->accept();
	closeLogStruct( 2);
	return (acceptType != 0);
}

bool WidgetWithDragAndDropBase::handleDropEvent( QWidget* this_, QDropEvent *event)
{
	WidgetVisitor visitor( this_);
	FormWidget* form = visitor.formwidget();
	openLogStruct( form->logId());
	openLogStruct( "drop");

	if (!form)
	{
		qWarning() << "drop rejected in a widget that is not a subwidget of a form";
		return false;
	}
	Qt::DropAction dropAction = event->dropAction();
	const char* dropActionStr = dropActionName( dropAction);

	if (!dropActionStr)
	{
		qCritical() << "[drag/drop handler] unknown drop action";
		closeLogStruct( 2);
		return true; //... tells that the event should not be interpreted differently
	}
	const QMimeData* mimedata = event->mimeData();
	if (!mimedata)
	{
		qCritical() << "[drag/drop handler] drop action missing data";
		closeLogStruct( 2);
		return true; //... tells that the event should not be interpreted differently
	}
	QByteArray dropdata = mimedata->data( WIDGETID_MIMETYPE);
	WidgetId dragWidgetId( dropdata);

	QVariant dropid = visitor.valueAt( event->pos());
	qDebug() << "[drag/drop handler] handle drop" << dropActionStr << "data" << dragWidgetId.toString() << "on" << dropid << "at" << event->pos();

	QString propname;
	if (dragWidgetId.objectName() == this_->objectName())
	{
		propname = QString("drop") + dropActionStr;
	}
	else
	{
		propname = QString("drop") + dropActionStr + ":" + dragWidgetId.objectName();
	}
	QVariant action = this_->property(propname.toLatin1());
	if (!action.isValid())
	{
		qCritical() << "[drag/drop handler] drop action not defined";
		closeLogStruct( 2);
		return true; //... tells that the event should not be interpreted differently
	}

	// Submit request
	if (sendDropRequest( this_, dragWidgetId, action.toString(), dropid))
	{
		// Refresh of involved widget via a datasignal
		handleDatasignal( visitor, "drop");
	}
	closeLogStruct( 2);
	return true; //... tells that the event should not be interpreted differently
}

bool WidgetWithDragAndDropBase::sendDropRequest( QWidget* dropWidget, const WidgetId& dragWidgetid, const QString& action, const QVariant& dropvalue)
{
	WidgetVisitor visitor( dropWidget, WidgetVisitor::AllowUndefDynPropsInit);

	visitor.defineLink( "dragobj", dragWidgetid.toString());
	visitor.setProperty( "dropid", dropvalue);

	// ... submit request
	qDebug() << "[drag/drop handler] submit drop request" << action;
	WidgetRequest request = getActionRequest( visitor, action, m_debug);
	if (!request.content.isEmpty())
	{
		if (m_dataLoader)
		{
			m_dataLoader->datarequest( request.header.command.toString(), request.header.toString(), request.content);
			return true;
		}
		else
		{
			qCritical() << "no data loader defined. cannot send drop request";
		}
	}
	else
	{
		qWarning() << "action for drop request is empty. No drop action performed";
	}
	return false;
}


