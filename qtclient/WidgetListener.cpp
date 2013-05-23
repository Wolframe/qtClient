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
#include "WidgetListener.hpp"
#include "WidgetEnabler.hpp"
#include "WidgetVisitorStateConstructor.hpp"
#include "WidgetVisitor.hpp"
#include "WidgetMessageDispatcher.hpp"
#include "FormWidget.hpp"
#include "FormCall.hpp"
#include <QAbstractButton>
#include <QAbstractScrollArea>
#include <QMenu>
#include <QDebug>
#include <QMessageBox>

void WidgetListenerImpl::blockSignals( bool v)
{
	QObject* object = m_state->widget();
	if (object) object->blockSignals( v);
}


bool WidgetListenerImpl::hasDataSignals( const QWidget* widget_)
{
	foreach (const QByteArray& prop, widget_->dynamicPropertyNames())
	{
		if (prop.startsWith( "datasignal:") && widget_->property( prop).isValid()) return true;
	}
	return false;
}

WidgetListenerImpl::WidgetListenerImpl( QWidget* widget_, DataLoader* dataLoader_)
	:WidgetListener()
	,m_state(createWidgetVisitorObject(widget_))
	,m_dataLoader(dataLoader_)
	,m_debug(false)
	,m_hasContextMenu(false)
{
	if (widget_->property( "contextmenu").isValid())
	{
		widget_->setContextMenuPolicy( Qt::CustomContextMenu);
		connect( widget_, SIGNAL( customContextMenuRequested( const QPoint&)),
			this, SLOT( showContextMenu( const QPoint&)));
		m_hasContextMenu = true;
	}
}

void WidgetListenerImpl::setDebug( bool v)
{
	if (!m_hasContextMenu)
	{
		if (v && !m_debug)
		{
			m_state->widget()->setContextMenuPolicy( Qt::CustomContextMenu);
			connect( m_state->widget(), SIGNAL( customContextMenuRequested( const QPoint&)),
				this, SLOT( showContextMenu( const QPoint&)));
		}
		else if (!v && m_debug)
		{
			m_state->widget()->setContextMenuPolicy( Qt::NoContextMenu);
			disconnect( m_state->widget(), SIGNAL( customContextMenuRequested( const QPoint&)),
				this, SLOT( showContextMenu( const QPoint&)));
		}
	}
	m_debug = v;
}

void WidgetListenerImpl::trigger_reload( const QString& slotname, QWidget* receiver)
{
	WidgetVisitor visitor( receiver, WidgetVisitor::None);
	qDebug() << "reload triggered of" << visitor.className() << visitor.objectName() << "on dataslot" << slotname;
	visitor.readAssignments();

	QAbstractButton* button = qobject_cast<QAbstractButton*>( receiver);
	if (button)
	{
		button->toggle();
		button->click();
	}
	QVariant actiondef;
	actiondef = receiver->property( QByteArray("action:") + slotname.toLatin1());
	if (!actiondef.isValid())
	{
		actiondef = receiver->property( "action");
	}
	if (actiondef.isValid())
	{
		WidgetRequest domload = getWidgetRequest( visitor, actiondef.toString(), m_debug);
		if (!domload.content.isEmpty())
		{
			m_dataLoader->datarequest( domload.cmd, domload.tag, domload.content);
		}
	}
}

QList<QWidget*> WidgetListenerImpl::get_forward_receivers( QWidget* receiver)
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
					foreach (const Receiver& forward_rcv, sndvisitor.get_datasignal_receivers( forward_rcvid.trimmed()))
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

void WidgetListenerImpl::handleDataSignal( DataSignalType dt)
{
	WidgetVisitor tv( m_state, WidgetVisitor::None);
	typedef QPair<QString,QWidget*> Receiver;
	qDebug() << "handle datasignal [" << dataSignalTypeName( dt) << "]";

	foreach (const Receiver& receiver, tv.get_datasignal_receivers( dt))
	{
		trigger_reload( receiver.first, receiver.second);
		foreach (QWidget* fwd, get_forward_receivers( receiver.second))
		{
			trigger_reload( receiver.first, fwd);
		}
	}
}

static bool widgetHasActions( QWidget* widget)
{
	if (widget->property( "action").isValid()) return true;
	if (widget->property( "form").isValid()) return true;

	foreach (const QByteArray& prop, widget->dynamicPropertyNames())
	{
		if (prop.startsWith( "action:")) return true;
		if (prop.startsWith( "form:")) return true;
	}
	return false;
}

static QString widgetText( QWidget* widget, const QString& menuitem=QString())
{
	QString text;
	QVariant action;
	QVariant form;
	WidgetVisitor visitor( widget, WidgetVisitor::None);
	QList<QString> condprops;
	WidgetRequest request;
	FormCall formCall;
	QString title;
	bool enabled = true;
	bool showFormDecl = true;

	if (menuitem.isEmpty())
	{
		action = widget->property( "action");
		form = visitor.property( "form");
		if (action.isValid())
		{
			foreach (const QString& cond, getActionRequestProperties( visitor))
			{
				if (!condprops.contains(cond))
				{
					condprops.push_back( cond);
				}
			}
			request = getActionRequest( visitor, action, true);
		}
		if (qobject_cast<QPushButton*>( widget))
		{
			title = "on click";
		}
		else
		{
			title = "for domain load";
			showFormDecl = false;
		}
	}
	else
	{
		action = widget->property( QByteArray("action:") + menuitem.toLatin1());
		form = visitor.property( QString("form:") + menuitem);
		if (action.isValid())
		{
			foreach (const QString& cond, getMenuActionRequestProperties( visitor, menuitem))
			{
				if (!condprops.contains(cond))
				{
					condprops.push_back( cond);
				}
			}
			request = getActionRequest( visitor, action, true, menuitem);
		}
		title = QString("for menu item '") + menuitem + "'";
	}
	if (!condprops.isEmpty())
	{
		text.append( "Condition variables ");
		text.append( title);
		text.append( ":\n   ");
		int idx = 0;
		foreach (const QString& cond, condprops)
		{
			if (idx++) text.append( ", ");
			text.append( cond);
			if (visitor.property( cond).isValid())
			{
				text.append( " [yes]");
			}
			else
			{
				text.append( " [no]");
				enabled = false;
			}
		}
		text.append( "\n");
	}
	if (form.isValid())
	{
		foreach (const QString& cond, getFormCallProperties( form.toString()))
		{
			if (!condprops.contains(cond))
			{
				condprops.push_back( cond);
			}
		}
		formCall.init( form.toString());
	}
	if (action.isValid())
	{
		if (enabled)
		{
			text.append( "Request ");
			text.append( title);
			text.append( ":\n");
			text.append( request.content);
		}
		else
		{
			text.append( "No action (not all conditions are met)\n");
		}
	}
	else
	{
		text.append( "No action defined ");
		text.append( title);
		text.append( "\n");
	}
	if (form.isValid())
	{
		if (enabled)
		{
			text.append( "Form ");
			text.append( title);
			text.append( " is '");
			text.append( formCall.name());

			if (formCall.parameter().isEmpty())
			{
				text.append( "' without parameters\n");
			}
			else
			{
				text.append( "':\n   Parameter ");
				int idx = 0;
				foreach (const FormCall::Parameter& param, formCall.parameter())
				{
					if (idx++)
					{
						text.append( "; ");
					}
					text.append( param.first);
					text.append( " = '");
					text.append( param.second.toString());
					text.append( "'");
				}
				text.append( "\n");
			}
		}
		else if (showFormDecl)
		{
			text.append( "No new form ");
			text.append( title);
			text.append(" (not all conditions are met)\n");
		}
	}
	else if (showFormDecl)
	{
		text.append( "No form defined ");
		text.append( title);
		text.append( "\n");
	}
	return text;
}

void WidgetListenerImpl::handleShowContextMenu( const QPoint& pos)
{
	QPoint globalPos;
	QWidget* widget = m_state->widget();
	if (qobject_cast<QAbstractScrollArea*>(widget))
	{
		QAbstractScrollArea* as = qobject_cast<QAbstractScrollArea*>( widget);
		globalPos = as->viewport()->mapToGlobal( pos);
	}
	else
	{
		globalPos = widget->mapToGlobal( pos);
	}
	QMenu menu;
	WidgetVisitor visitor( m_state, WidgetVisitor::None);
	QVariant contextmenudef_p( visitor.property( "contextmenu"));
	QList<QString> contextmenudef( contextmenudef_p.toString().split(','));
	int nofMenuEntries = 0;

	if (contextmenudef_p.isValid())
	{
		foreach (const QString& item, contextmenudef)
		{
			QString itemname = item.trimmed();
			if (itemname.isEmpty())
			{
				menu.addSeparator();
			}
			else
			{
				// defined context menu item:
				QByteArray prop( QByteArray( "contextmenu:") + itemname.toLatin1());
				QVariant actiontext( widget->property( prop));
				QAction* action;
				if (actiontext.isValid())
				{
					action = menu.addAction( actiontext.toString());
				}
				else
				{
					action = menu.addAction( item);
				}
				action->setData( QVariant( itemname));
				++nofMenuEntries;

				// check if menu item should be enabled or not:
				QList<QString> menuprops;
				foreach (const QString& menuprop, getMenuActionRequestProperties( visitor, itemname))
				{
					if (!menuprops.contains( menuprop)) menuprops.push_back( menuprop);
				}
				foreach (const QString& menuprop, getFormCallProperties( widget->property( QByteArray("form:") + itemname.toLatin1()).toString()))
				{
					if (!menuprops.contains( menuprop)) menuprops.push_back( menuprop);
				}
				bool enabled = true;
				foreach (const QString& menuprop, menuprops)
				{
					if (!visitor.property( menuprop).isValid())
					{
						qDebug() << "Menu item" << itemname << "disabled because of undefined property" << menuprop;
						enabled = false;
					}
				}
				action->setEnabled( enabled);
			}
		}
	}
	if (m_debug && widgetHasActions( widget))
	{
		if (nofMenuEntries)
		{
			menu.addSeparator();
		}
		QAction* action = menu.addAction( "Debug: Inspect commands");
		action->setData( QVariant( 1));
		++nofMenuEntries;
	}
	if (nofMenuEntries)
	{
		QAction* selectedItem = menu.exec( globalPos);
		if (selectedItem)
		{
			FormWidget* form = visitor.formwidget();
			if (!form)
			{
				qCritical() << "no form associated with widget with context menu action";
				return;
			}
			QVariant action = selectedItem->data();
			if (!action.isValid())
			{
				qCritical() << "no data associated context menu action";
				return;
			}
			if (action.type() == QVariant::Int)
			{
				QString debugtext = widgetText( widget);
				foreach (const QString& item, contextmenudef)
				{
					QString itemname = item.trimmed();
					if (!itemname.isEmpty())
					{
						if (!debugtext.isEmpty())
						{
							debugtext.append( "-\n");
						}
						debugtext.append( widgetText( widget, itemname));
					}
				}
				QMessageBox msg( widget);
				msg.setText( debugtext);
				msg.exec();
			}
			else
			{
				form->executeMenuAction( widget, action.toString());
			}
		}
	}
}





