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

static bool parseOperator( WidgetEnablerImpl::Operator& op, QString::const_iterator& ci, const QString::const_iterator end)
{
	op = WidgetEnablerImpl::Valid;
	QString::const_iterator xi = ci;
	for (; xi != end && xi->isSpace(); ++xi);
	if (xi == end) return false;

	if (*xi == '>')
	{
		++xi;
		if (xi != end && *xi == '=')
		{
			op = WidgetEnablerImpl::Ge;
			++xi;
		}
		else
		{
			op = WidgetEnablerImpl::Gt;
		}
	}
	else if (*xi == '<')
	{
		++xi;
		if (xi != end && *xi == '=')
		{
			op = WidgetEnablerImpl::Le;
			++xi;
		}
		else if (xi != end && *xi == '>')
		{
			op = WidgetEnablerImpl::Ne;
			++xi;
		}
		else
		{
			op = WidgetEnablerImpl::Lt;
		}
	}
	else if (*xi == '!')
	{
		++xi;
		if (xi != end && *xi == '=')
		{
			op = WidgetEnablerImpl::Ne;
			++xi;
		}
	}
	else if (*xi == '=')
	{
		++xi;
		if (xi != end && *xi == '=')
		{
			++xi;
		}
		op = WidgetEnablerImpl::Eq;
	}
	if (op != WidgetEnablerImpl::Valid)
	{
		ci = xi;
		return true;
	}
	return false;
}

bool parseCondVariable( QString& var, QString::const_iterator& ci, const QString::const_iterator end)
{
	QString::const_iterator xi = ci;
	for (; xi != end && xi->isSpace(); ++xi);
	if (xi == end) return false;

	var.clear();
	if (*xi == '{')
	{
		for (++xi; xi != end && *xi != '{' && *xi != '}'; ++xi)
		{
			var.push_back( *xi);
		}
		if (xi == end || *xi != '}')
		{
			qCritical() << "illegal variable reference in conditional expression:" << QString( ci, end-ci);
			return false;
		}
		else
		{
			++xi;
		}
	}
	else
	{
		qCritical() << "expected variable refernce in conditional expression:" << QString( ci, end-ci);
	}
	ci = xi;
	return true;
}

static bool getOperand( QVariant& opr, QString::const_iterator& xi, const QString::const_iterator xe)
{
	opr = QVariant();
	QString rt;
	for (; xi != xe && xi->isSpace(); ++xi);
	if (xi != xe)
	{
		if (*xi == '\'' || *xi == '"')
		{
			QChar eb = *xi;
			for (++xi; xi != xe && *xi != eb; ++xi)
			{
				rt.push_back( *xi);
			}
			if (xi == xe)
			{
				qCritical() << "string not terminated in expression";
			}
			else
			{
				++xi;
			}
			opr = QVariant(rt);
			return true;
		}
		else
		{
			for (; xi != xe && xi->isSpace(); ++xi);
			for (; xi != xe && !xi->isSpace() && *xi != '=' && *xi != '!' && *xi != '>' && *xi != '<'; ++xi)
			{
				rt.push_back( *xi);
			}
			if (rt.isEmpty()) return false;
			opr = QVariant(rt);
			return true;
		}
	}
	return false;
}

bool WidgetEnablerImpl::parseCondition( Condition& cond, const QString& expr)
{
	QString condvar;
	Operator condop;
	QVariant condopr;
	QString::const_iterator xi = expr.begin(), xe = expr.end();

	if (!parseCondVariable( condvar, xi, xe))
	{
		if (xi == xe) qCritical() << "empty condition expression";
		return false;
	}
	if (!parseOperator( condop, xi, xe))
	{
		if (xi == xe)
		{
			cond = Condition( condvar);
#ifdef WOLFRAME_LOWLEVEL_DEBUG
	qDebug() << "parsed conditional expression" << condvar << "(is valid)";
#endif
			return true;
		}
		qCritical() << "failed to parse condition expression operator:" << expr;
		return false;
	}
	if (!getOperand( condopr, xi, xe))
	{
		qCritical() << "failed to parse condition expression operand:" << expr;
		return false;
	}
	for (; xi != xe && xi->isSpace(); ++xi);
	if (xi != xe)
	{
		qCritical() << "unexpected characters at end of condition expression:" << expr;
		return false;
	}
	cond = Condition( condvar, condop, condopr);

#ifdef WOLFRAME_LOWLEVEL_DEBUG
	qDebug() << "parsed conditional expression" << condvar << operatorName(condop) << condopr;
#endif
	return true;
}

WidgetEnablerImpl::WidgetEnablerImpl( QWidget* widget_, const QList<Trigger>& trigger_)
	:WidgetEnabler()
	,m_state(createWidgetVisitorObject(widget_))
	,m_trigger(trigger_)
{}

void WidgetEnablerImpl::blockSignals( bool v)
{
	QObject* object = m_state->widget();
	if (object)
	{
#ifdef WOLFRAME_LOWLEVEL_DEBUG
		qDebug() << (v?"blocking":"unblocking") << "signals in WidgetEnblerImpl for object" << object->metaObject()->className() << object->objectName();
#endif
		object->blockSignals( v);
	}
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
		qDebug() << "handle changed of widget" << widget->objectName() << "check condition";
		// Aba: prop run away, removed
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
		QVariant propval = visitor.property( trg.condition.property);
		QVariant cmpval = trg.condition.value;

		bool res = false;
		switch (trg.condition.op)
		{
			case Valid: res = propval.isValid(); break;
			case Eq: res = (propval.toString().trimmed() == cmpval.toString().trimmed()); break;
			case Ne: res = (propval.toString().trimmed() != cmpval.toString().trimmed()); break;
			case Lt: res = (propval.toInt() < cmpval.toInt()); break;
			case Gt: res = (propval.toInt() > cmpval.toInt()); break;
			case Le: res = (propval.toInt() <= cmpval.toInt()); break;
			case Ge: res = (propval.toInt() >= cmpval.toInt()); break;
		}
		if (!res)
		{
			if (visitor.getPropertyOwnerWidget( trg.condition.property))
			{
				if (trg.condition.op != Valid)
				{
					qDebug() << "widget" << widget->objectName() << "not set to" << statename << "because condition" << trg.condition.property << "is not met (condition not valid: " << propval << operatorName(trg.condition.op) << cmpval << ")";
				}
				else
				{
					qDebug() << "widget" << widget->objectName() << "not set to" << statename << "because condition" << trg.condition.property << "is not met (condition not valid: property undefined)";
				}
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


