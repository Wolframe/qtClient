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
#include "WidgetDataSignal.hpp"
#include "WidgetId.hpp"
#include "WidgetRequest.hpp"
#include "FormWidget.hpp"

#undef WOLFRAME_LOWLEVEL_DEBUG

static bool nodeProperty_hasWidgetId( const QWidget* widget, const QVariant& cond)
{
	return widgetIdMatches( cond.toString(), widget);
}

static bool nodeProperty_hasDataSlot( const QWidget* widget, const QVariant& cond)
{
	QVariant dataslots = widget->property( "dataslot");
	int idx = 0;
	while ((idx=dataslots.toString().indexOf( cond.toString(), idx)) >= 0)
	{
		idx += cond.toString().length();
		QString dd = dataslots.toString();
		if (dd.size() == idx || dd.at(idx) == ' ' || dd.at(idx) == ','  || dd.at(idx) == '[')
		{
			return true;
		}
	}
	return false;
}

static QVariant getDatasignalSender( QWidget* widget, const QVariant& cond)
{
	WidgetVisitor visitor( widget, WidgetVisitor::None);
	QVariant dataslots = visitor.property( "dataslot");
	int idx = 0;
	QString dd = dataslots.toString();
	while ((idx=dd.indexOf( cond.toString(), idx)) >= 0)
	{
		idx += cond.toString().length();
		if (dd.size() == idx || dd.at(idx) == ' ' || dd.at(idx) == ','  || dd.at(idx) == '[') break;
	}
	if (idx >= 0)
	{
		while (idx < dd.size() && dd.at(idx) == ' ') ++idx;
		if (idx < dd.size() && dd.at(idx) == '[')
		{
			int endidx = dd.indexOf( ']', ++idx);
			if (endidx >= idx)
			{
				QVariant rt( dd.mid( idx, endidx-idx).trimmed());
				return rt;
			}
		}
	}
	return QVariant();
}

QList<DataSignalReceiver> getDataSignalReceivers( WidgetVisitor& visitor, const QString& receiverAddr, bool withThis)
{
	QList<DataSignalReceiver> rt;
	QWidget* widget = visitor.widget();

#ifdef WOLFRAME_LOWLEVEL_DEBUG
	qDebug() << "[data signal handler] find receiver" << widget->objectName() << receiverAddr;
#endif
	QWidget* rcvwidget;
	QList<QWidget*> wl;
	int atidx = receiverAddr.indexOf('@');
	QString slotname;
	QString address;
	if (atidx >= 0)
	{
		slotname = receiverAddr.mid( 0, atidx).trimmed();
		address = receiverAddr.mid( atidx+1, receiverAddr.size() - (atidx+1)).trimmed();
	}
	else
	{
		address = receiverAddr;
	}
	if (WidgetId::isValid( address))
	{
		WidgetVisitor mainvisitor( visitor.uirootwidget(), WidgetVisitor::None);
		wl.append( mainvisitor.findSubNodes( nodeProperty_hasWidgetId, address));
		foreach (QWidget* rcvwidget, wl)
		{
#ifdef WOLFRAME_LOWLEVEL_DEBUG
			qDebug() << "[data signal handler] found widget by address" << rcvwidget->objectName() << address;
#endif
			rt.push_back( DataSignalReceiver( slotname, rcvwidget));
		}
	}
	else if (address.indexOf('.') >= 0)
	{
		rcvwidget = visitor.get_widget_reference( address);
		if (rcvwidget)
		{
#ifdef WOLFRAME_LOWLEVEL_DEBUG
			qDebug() << "[data signal handler] found widget reference" << rcvwidget->objectName() << address;
#endif
			rt.push_back( DataSignalReceiver( slotname, rcvwidget));
		}
		else
		{
			qCritical() << "[data signal handler] unable to resolve local widget reference:" << address;
		}
	}
	else if ((rcvwidget = visitor.get_widget_reference( address)) != 0)
	{
#ifdef WOLFRAME_LOWLEVEL_DEBUG
		qDebug() << "[data signal handler] found widget reference" << rcvwidget->objectName() << address;
#endif
		rt.push_back( DataSignalReceiver( slotname, rcvwidget));
	}
	else
	{
		if (slotname.isEmpty())
		{
			slotname = address;
		}
		WidgetVisitor mainvisitor( visitor.uirootwidget(), WidgetVisitor::None);
		foreach (QWidget* rcvwidget, mainvisitor.findSubNodes( nodeProperty_hasDataSlot, address))
		{
			if (withThis || rcvwidget != widget)
			{
				QVariant sendercond = getDatasignalSender( rcvwidget, address);
				if (sendercond.isValid())
				{
					if (sendercond.toString() == visitor.widgetid())
					{
#ifdef WOLFRAME_LOWLEVEL_DEBUG
						qDebug() << "[data signal handler] found widget by data slot identifier with sender id" << rcvwidget->objectName() << getWidgetId(rcvwidget);
#endif
						rt.push_back( DataSignalReceiver( slotname, rcvwidget));
					}
				}
				else
				{
#ifdef WOLFRAME_LOWLEVEL_DEBUG
					qDebug() << "[data signal handler] found widget by data slot identifier" << rcvwidget->objectName() << getWidgetId(rcvwidget);
#endif
					rt.push_back( DataSignalReceiver( slotname, rcvwidget));
				}
			}
		}
	}
	return rt;
}

QList<QString> parseDataSignalList( const QString& datasiglist)
{
	QList<QString> rt;
	if (datasiglist.trimmed().isEmpty()) return rt;

	foreach (const QString& vv, datasiglist.split(','))
	{
		rt.push_back( vv.trimmed());
	}
	return rt;
}

static QList<QWidget*> getTransitiveCoverOfSignaledReceivers( QWidget* receiver)
{
	QList<QWidget*> forwardlist;
	int forwardlistidx = 0, forwardlistsize = 0;
	forwardlist.push_back( receiver);

	do
	{
		forwardlistsize = forwardlist.size();
		for (;forwardlistidx < forwardlistsize; ++forwardlistidx)
		{
			QWidget* forwardsnd = forwardlist.at( forwardlistidx);
			if (forwardsnd->property("datasignal:signaled").isValid())
			{
				WidgetVisitor sndvisitor( forwardsnd, WidgetVisitor::None);
				foreach (const QString& forward_rcvid, sndvisitor.property( "datasignal:signaled").toString().split(','))
				{
					typedef QPair<QString,QWidget*> Receiver;
					foreach (const Receiver& forward_rcv, getDataSignalReceivers( sndvisitor, forward_rcvid.trimmed(), false))
					{
						if (!forwardlist.contains( forward_rcv.second))
						{
							forwardlist.push_back( forward_rcv.second);
						}
					}

				}
			}
		}
	}
	while (forwardlistsize < forwardlist.size());

	forwardlist.removeFirst();
	return forwardlist;
}

void DataSignalHandler::trigger( const QString& signame, QWidget* receiver)
{
	if (signame == "close")
	{
		FormWidget* formwidget = qobject_cast<FormWidget*>( receiver);
		if (!formwidget) formwidget = qobject_cast<FormWidget*>( receiver->parent());
		if (formwidget)
		{
			formwidget->triggerClose();
		}
	}
	else
	{
		WidgetVisitor visitor( receiver);
		qDebug() << "signal" << signame << "triggers reload of" << receiver->objectName();
		visitor.readAssignments();

		QAbstractButton* button = qobject_cast<QAbstractButton*>( receiver);
		if (button)
		{
			button->toggle();
			button->click();
		}
		QVariant actiondef;
		actiondef = receiver->property( QByteArray("action:") + signame.toLatin1());
		if (!actiondef.isValid())
		{
			actiondef = receiver->property( "action");
		}
		if (actiondef.isValid())
		{
			WidgetRequest domload = getDataloadRequest( visitor, actiondef.toString(), m_debug, signame);
			if (!domload.content.isEmpty())
			{
				receiver->setProperty( "_w_state", visitor.getState());
				m_dataLoader->datarequest( domload.header.command.toString(), domload.header.toString(), domload.content);
			}
		}
		if (signame != "signaled")
		{
			foreach (QWidget* fwd, getTransitiveCoverOfSignaledReceivers( receiver))
			{
				trigger( "signaled", fwd);
			}
		}
	}
}

