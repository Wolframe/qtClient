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
#include "WidgetDragAndDrop.hpp"
#include "WidgetId.hpp"
#include "WidgetRequest.hpp"
#include "WidgetDataSignal.hpp"
#include "FormWidget.hpp"
#include "debugview/DebugLogTree.hpp"
#include <QDrag>
#include <QMimeData>

#define WOLFRAME_LOWLEVEL_DEBUG

static const char* dropActionName( const Qt::DropAction& dropAction)
{
	switch (dropAction)
	{
		case Qt::CopyAction: return "copy";
		case Qt::MoveAction: return "move";
		default: return 0;
	}
}

void WidgetWithDragAndDropBase::handleDatasignal( WidgetVisitor& visitor, const char* sigpropname)
{
	DataSignalHandler dshandler( m_dataLoader, m_debug);
	QList<DataSignalReceiver> rcvlist;
	QVariant prop = visitor.property( sigpropname);
	if (prop.isValid())
	{
		foreach (const QString& rcvaddr, parseDataSignalList( prop.toString()))
		{
			rcvlist.append( getDataSignalReceivers( visitor, rcvaddr));
		}
		foreach (const DataSignalReceiver& receiver, rcvlist)
		{
			dshandler.trigger( receiver.first, receiver.second);
			qDebug() << "[drag/drop handler] refresh of" << getWidgetId( receiver.second) << "invoked by" << sigpropname;
		}
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
	
#ifdef WOLFRAME_LOWLEVEL_DEBUG
	qDebug() << "[drag/drop handler] handle drag pick" << this_->objectName();
#endif
#ifdef WOLFRAME_LOWLEVEL_DEBUG
	qDebug() << "[drag/drop handler] flag accept drops" << (this_->acceptDrops()?"TRUE":"FALSE");
	qDebug() << "[drag/drop handler] drag item value" << visitor.valueAt( event->pos());
#endif
	event->accept();
	drag = new QDrag( this_);
	QMimeData *mimeData = new QMimeData;
	mimeData->setData( WIDGETID_MIMETYPE, visitor.widgetid().toLatin1());
	drag->setMimeData( mimeData);
	drag->setPixmap( QPixmap( ":/images/16x16/copy.png"));
	closeLogStruct(2);
	Qt::DropAction dropAction = drag->exec( Qt::CopyAction | Qt::MoveAction);
	//... drag->exec is blocking till end of drop. so the following code is executed after !

	const char* dropActionStr = dropActionName(dropAction);
	openLogStruct( visitor.formwidget()->logId());
	openLogStruct( "drag");
	if (dropAction == Qt::MoveAction)
	{
		handleDatasignal( visitor, "datasignal:drag");
	}
	qDebug() << "[drag/drop handler] drop action" << (dropActionStr?dropActionStr:"ignore") << visitor.widgetid();
	closeLogStruct(2);
	return true;
}

bool WidgetWithDragAndDropBase::handleDragEnterEvent( QWidget* this_, QDragEnterEvent* event)
{
	bool rt = false;
	WidgetVisitor visitor( this_);
	FormWidget* form = visitor.formwidget();
	if (!form) return false;

	openLogStruct( form->logId());
	openLogStruct( "drag");

#ifdef WOLFRAME_LOWLEVEL_DEBUG
	qDebug() << "[drag/drop handler] handle drag enter" << this_->objectName() << event->mimeData()->formats();
#endif
	if (event->mimeData()->hasFormat( WIDGETID_MIMETYPE))
	{
		event->accept();

#ifdef WOLFRAME_LOWLEVEL_DEBUG
		qDebug() << "[drag/drop handler] handle drag enter for mime type" << WIDGETID_MIMETYPE << "on" << this_->objectName();
#endif
		WidgetId widgetId( this_);

		if (widgetId.objectName() == this_->objectName())
		{
			if (this_->property("dropmove").isValid())
			{
				if (event->dropAction() == Qt::IgnoreAction)
				{
					event->setDropAction( Qt::MoveAction);
					event->accept();
					qDebug() << "[drag/drop handler] drag enter accept move forced in itself at" << event->pos() << (event->isAccepted()?"ACCEPTED":"REJECTED");
					rt = true;
				}
				else if (event->dropAction() == Qt::MoveAction)
				{
					event->acceptProposedAction();
					qDebug() << "[drag/drop handler] drag enter accept move proposed in itself at" << event->pos() << (event->isAccepted()?"ACCEPTED":"REJECTED");
					rt = true;
				}
			}
			if (!rt && this_->property("dropcopy").isValid())
			{
				if (event->dropAction() == Qt::IgnoreAction)
				{
					event->setDropAction( Qt::CopyAction);
					event->accept();
					qDebug() << "[drag/drop handler] drag enter accept copy forced in itself at" << event->pos() << (event->isAccepted()?"ACCEPTED":"REJECTED");
					rt = true;
				}
				else if (event->dropAction() == Qt::CopyAction)
				{
					event->acceptProposedAction();
					qDebug() << "[drag/drop handler] drag enter accept copy proposed in itself at" << event->pos() << (event->isAccepted()?"ACCEPTED":"REJECTED");
					rt = true;
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
					event->setDropAction( Qt::MoveAction);
					event->accept();
					qDebug() << "[drag/drop handler] drag enter accept move forced to" << this_->objectName() << "at" << event->pos() << (event->isAccepted()?"ACCEPTED":"REJECTED");
					rt = true;
				}
				else if (event->dropAction() == Qt::MoveAction)
				{
					event->acceptProposedAction();
					qDebug() << "[drag/drop handler] drag enter accept move proposed to" << this_->objectName() << "at" << event->pos() << (event->isAccepted()?"ACCEPTED":"REJECTED");
					rt = true;
				}
			}
			if (!rt && this_->property(propname_copy.toLatin1()).isValid())
			{
				if (event->dropAction() == Qt::IgnoreAction)
				{
					event->setDropAction( Qt::CopyAction);
					event->accept();
					qDebug() << "[drag/drop handler] drag enter accept copy forced to" << this_->objectName() << "at" << event->pos() << (event->isAccepted()?"ACCEPTED":"REJECTED");
					rt = true;
				}
				else if (event->dropAction() == Qt::CopyAction)
				{
					event->acceptProposedAction();
					qDebug() << "[drag/drop handler] drag enter accept copy proposed to" << this_->objectName() << "at" << event->pos() << (event->isAccepted()?"ACCEPTED":"REJECTED");
					rt = true;
				}
			}
		}
	}
	closeLogStruct( 2);
	return rt;
}

bool WidgetWithDragAndDropBase::handleDropEvent( QWidget* this_, QDropEvent *event)
{
	WidgetVisitor visitor( this_);
	FormWidget* form = visitor.formwidget();
	if (!form) return false;

	openLogStruct( form->logId());
	openLogStruct( "drop");

#ifdef WOLFRAME_LOWLEVEL_DEBUG
	qDebug() << "[drag/drop handler] handle drop" << this_->objectName();
#endif
	Qt::DropAction dropAction = event->dropAction();
	const char* dropActionStr = dropActionName( dropAction);

	if (!dropActionStr)
	{
		qCritical() << "[drag/drop handler] unknown drop action";
		closeLogStruct( 2);
		return false;
	}
	const QMimeData* mimedata = event->mimeData();
	if (!mimedata)
	{
		qCritical() << "[drag/drop handler] missing drop data";
		closeLogStruct( 2);
		return false;
	}
	QByteArray dropdata = mimedata->data( WIDGETID_MIMETYPE);
	WidgetId dragWidgetId( dropdata);
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
		return false;
	}

	// ... submit request
	qDebug() << "[drag/drop handler] signal drop request" << action.toString();
	bool rt = sendDropRequest( this_, dragWidgetId, action.toString(), visitor.valueAt( event->pos()));
	if (rt)
	{
		handleDatasignal( visitor, "datasignal:drop");
	}
	closeLogStruct( 2);
	return rt;
}

bool WidgetWithDragAndDropBase::sendDropRequest( QWidget* dropWidget, const WidgetId& dragWidgetid, const QString& action, const QVariant& dropvalue)
{
	WidgetVisitor visitor( dropWidget);

	qDebug() << "[drag/drop handler] define 'drag' link in drop visitor" << dragWidgetid.toString();
	visitor.defineLink( "dragobj", dragWidgetid.toString());
	visitor.setProperty( "dropobj", dropvalue);

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
	return false;
}


