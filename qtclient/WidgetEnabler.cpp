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
#include "WidgetEnabler.hpp"
#include "WidgetVisitorObject.hpp"
#include "WidgetVisitor.hpp"
#include "WidgetVisitorStateConstructor.hpp"
#include <QDebug>

#undef WOLFRAME_LOWLEVEL_DEBUG

WidgetEnablerImpl::WidgetEnablerImpl( QWidget* widget_, const QList<Trigger>& trigger_)
	:WidgetEnabler()
	,m_state(createWidgetVisitorObject(widget_))
	,m_trigger(trigger_)
{}

void WidgetEnablerImpl::blockSignals( bool v)
{
	QObject* object = m_state->widget();
	if (object) object->blockSignals( v);
}

void WidgetEnablerImpl::handle_changed()
{
	QWidget* widget = m_state->widget();
	if (!widget)
	{
		qCritical() << "enabler has no widget defined";
		return;
	}
	WidgetVisitor visitor( m_state, WidgetVisitor::None);
	int enabled = 2;
	int disabled = 2;
	int hidden = 2;
	int visible = 2;
	const char* statename = stateName( Enabled);

	foreach (const Trigger& trg, m_trigger)
	{
#ifdef WOLFRAME_LOWLEVEL_DEBUG
		qDebug() << "handle changed of widget" << widget->objectName() << "check condition" << prop;
#endif
		int* setter = &enabled;
		statename = stateName( trg.state);

		switch (trg.state)
		{
			case Enabled: setter = &enabled; break;
			case Disabled: setter = &disabled; break;
			case Hidden: setter = &hidden; break;
			case Visible: setter = &visible; break;
		}
		if (trg.condition.value.isValid())
		{
			QString propval = visitor.property( trg.condition.property).toString().trimmed();
			QString cmpval = trg.condition.value.toString().trimmed();
			if (propval == cmpval)
			{
				if (visitor.getPropertyOwnerWidget( trg.condition.property))
				{
					qDebug() << "widget" << widget->objectName() << "not set to" << statename << "because condition" << trg.condition.property << "is not met (condition not valid: " << propval << "==" << cmpval << ")";
				}
				else
				{
					qDebug() << "widget" << widget->objectName() << "not set to" << statename << "because condition" << trg.condition.property << "is not met (condition not valid - owner undefined)";
				}
				*setter = 0;
			}
			else if (*setter == 2)
			{
				*setter = 1;
			}
		}
		else if (!visitor.property( trg.condition.property).isValid())
		{
			if (visitor.getPropertyOwnerWidget( trg.condition.property))
			{
				qDebug() << "widget" << widget->objectName() << "not set to" << statename << "because condition" << trg.condition.property << "is not met (condition not valid: property undefined)";
			}
			else
			{
				qDebug() << "widget" << widget->objectName() << "not set to" << statename << "because condition" << trg.condition.property << "is not met (condition not valid - property owner undefined)";
			}
			*setter = 0;
		}
		else if (*setter == 2)
		{
			*setter = 1;
		}
	}
	if (enabled != 2)
	{
		if (enabled) qDebug() << "widget" << widget->objectName() << "set to enabled (all conditions met)";
		widget->setEnabled( enabled != 0);
	}
	if (disabled != 2)
	{
		if (disabled) qDebug() << "widget" << widget->objectName() << "set to disabled (all conditions met)";
		widget->setDisabled( disabled != 0);
	}
	if (hidden != 2)
	{
		if (hidden) qDebug() << "widget" << widget->objectName() << "set to hidden (all conditions met)";
		widget->setHidden( hidden != 0);
	}
	if (visible != 2)
	{
		if (visible) qDebug() << "widget" << widget->objectName() << "set to visible (all conditions met)";
		widget->setVisible( visible != 0);
	}
}


