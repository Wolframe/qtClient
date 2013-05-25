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

#ifndef _WIDGET_Enabler_HPP_INCLUDED
#define _WIDGET_Enabler_HPP_INCLUDED
#include "WidgetVisitorObject.hpp"
#include <QObject>
#include <QWidget>
#include <QList>
#include <QString>
#include <QSharedPointer>

///\class WidgetEnabler
///\brief Implementation of WidgetEnabler
class WidgetEnablerImpl :public WidgetEnabler
{
public:
	enum State
	{
		Enabled,
		Disabled,
		Hidden,
		Visible
	};
	static const char* stateName( State i)
	{
		static const char* ar[] = {"Enabled","Disabled","Hidden","Visible"};
		return ar[i];
	}

	struct Condition
	{
		QString property;
		QVariant value;

		Condition();
		Condition( const Condition& o)
			:property(o.property),value(o.value){}
		Condition( const QString& property_, const QVariant& value_)
			:property(property_),value(value_){}
	};

	struct Trigger
	{
		State state;
		Condition condition;

		Trigger();
		Trigger( const Trigger& o)
			:state(o.state),condition(o.condition){}
		Trigger( const State& state_, const QString& property_, const QVariant& value_=QVariant())
			:state(state_), condition( property_, value_) {}
	};

	///\brief Constructor
	WidgetEnablerImpl( QWidget* widget_, const QList<Trigger>& trigger_);
	virtual ~WidgetEnablerImpl(){}

	QWidget* actionwidget() const			{return m_state->widget();}
	const QList<Trigger>& actiontrigger() const	{return m_trigger;}

	virtual void handle_changed();

	void blockSignals( bool v);

private:
	WidgetVisitorObjectR m_state;
	const QList<Trigger> m_trigger;
};

typedef QSharedPointer<WidgetEnablerImpl> WidgetEnablerR;

#endif

