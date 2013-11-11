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
#include "WidgetVisitor.hpp"
#include "WidgetId.hpp"
#include "WidgetRequest.hpp"
#include "WidgetVisitorStateConstructor.hpp"
#include "FileChooser.hpp"
#include "PictureChooser.hpp"
#include "FormWidget.hpp"
#include "debugview/DebugHelpers.hpp"

#include <QDebug>
#include <QXmlStreamWriter>
#include <QLineEdit>
#include <QComboBox>
#include <QPlainTextEdit>
#include <QCheckBox>
#include <QRadioButton>
#include <QListWidget>
#include <QTreeWidget>
#include <QTableWidget>
#include <QSharedPointer>
#include <QLayout>
#include <QLayout>
#include <cstring>

#undef WOLFRAME_LOWLEVEL_DEBUG
#ifdef WOLFRAME_LOWLEVEL_DEBUG
#define TRACE_STATUS( TITLE, CLASS, OBJ, NAME)		qDebug() << "widget visit state" << (TITLE) << (CLASS) << (OBJ) << (NAME);
#define TRACE_FETCH( TITLE, OBJ, NAME, VALUE)		qDebug() << "widget visit get" << (TITLE) << (OBJ) << (NAME) << "=" << shortenDebugMessageArgument(VALUE);
#define TRACE_ASSIGNMENT( TITLE, OBJ, NAME, VALUE)	qDebug() << "widget visit set" << (TITLE) << (OBJ) << (NAME) << "=" << shortenDebugMessageArgument(VALUE);
#define TRACE_ENTER( TITLE, CLASS, OBJ, NAME)		qDebug() << "widget visit enter" << (TITLE) << (CLASS) << (OBJ) << "into" << (NAME);
#define TRACE_LEAVE( TITLE)				qDebug() << "widget visit leave" << (TITLE);
#else
#define TRACE_STATUS( TITLE, CLASS, OBJ, NAME)
#define TRACE_FETCH( TITLE, OBJ, NAME, VALUE)
#define TRACE_ASSIGNMENT( TITLE, OBJ, NAME, VALUE)
#define TRACE_ENTER( TITLE, CLASS, OBJ, NAME)
#define TRACE_LEAVE( TITLE)
#endif

static void logError( const WidgetVisitor& visitor, const char* msg, const QString& arg)
{
	qCritical() << "error widget visitor" << (visitor.widget()?visitor.widget()->metaObject()->className():QString()) << visitor.widgetPath() << msg << ":" << arg;
}

static void logError( const WidgetVisitor& visitor, const char* msg)
{
	qCritical() << "error widget visitor" << (visitor.widget()?visitor.widget()->metaObject()->className():QString()) << visitor.widgetPath() << msg;
}

WidgetVisitor::State::State( const State& o)
	:m_obj(o.m_obj)
	,m_links(o.m_links)
	,m_assignments(o.m_assignments)
	,m_datasignals(o.m_datasignals)
	,m_dataslots(o.m_dataslots)
	,m_dynamicProperties(o.m_dynamicProperties)
	,m_multipart_entercnt(o.m_multipart_entercnt)
	,m_internal_entercnt(o.m_internal_entercnt)
	,m_blockSignals(o.m_blockSignals)
	,m_blockSignals_bak(o.m_blockSignals_bak)
{}

WidgetVisitor::State::State()
	:m_multipart_entercnt(1)
	,m_internal_entercnt(0)
	,m_blockSignals(false)
	,m_blockSignals_bak(false)
{}

WidgetVisitor::State::State( WidgetVisitorObjectR obj_, bool blockSignals_)
	:m_obj(obj_)
	,m_multipart_entercnt(1)
	,m_internal_entercnt(0)
	,m_blockSignals(blockSignals_)
	,m_blockSignals_bak(false)
{
	foreach (const QByteArray& prop, m_obj->widget()->dynamicPropertyNames())
	{
		if (prop.indexOf(':') >= 0)
		{
			if (prop.startsWith( "link:"))
			{
				QVariant link = m_obj->widget()->property( prop);
				m_links.push_back( LinkDef( prop.mid( 5, prop.size()-5), link.toString().trimmed()));
			}
			else if (prop.startsWith( "assign:"))
			{
				QVariant value = m_obj->widget()->property( prop);
				m_assignments.push_back( Assignment( prop.mid( 7, prop.size()-7), value.toString().trimmed()));
			}
			else if (prop.startsWith( "global:"))
			{
				QVariant value = m_obj->widget()->property( prop);
				m_globals.push_back( Assignment( prop.mid( 7, prop.size()-7), value.toString().trimmed()));
			}
			else if (prop.startsWith( "datasignal:"))
			{
				const char* signalname = (const char*)prop + 11/*std::strlen( "datasignal:")*/;
				QList<QString> values;
				foreach (const QString& vv, m_obj->widget()->property( prop).toString().trimmed().split(','))
				{
					values.push_back( vv.trimmed());
				}
				WidgetListener::DataSignalType dt;
				if (strcmp( signalname, "signaled") != 0)
				{
					// ... forward is handled differently
					if (WidgetListener::getDataSignalType( signalname, dt))
					{
						m_datasignals.id[ (int)dt] = values;
					}
					else
					{
						qCritical() << "error widget visitor state" << m_obj->widget()->metaObject()->className() << WidgetVisitor::widgetPath(m_obj->widget()) << ": defined unknown data signal name" << prop;
					}
				}
			}
		}
		if (!prop.startsWith( "_w_") && !prop.startsWith( "_q_"))
		{
			m_dynamicProperties.insert( prop, m_obj->widget()->property( prop));
		}
	}
	QVariant dataslots = m_obj->widget()->property( "dataslot");
	if (dataslots.isValid())
	{
		foreach (const QString& vv, dataslots.toString().trimmed().split(','))
		{
			m_dataslots.push_back( vv.trimmed());
		}
	}
	setWidgetId( m_obj->widget());

	if (m_blockSignals && !m_obj->widget()->signalsBlocked())
	{
#ifdef WOLFRAME_LOWLEVEL_DEBUG
		qDebug() << "block signals of" << m_obj->widget()->metaObject()->className() << m_obj->widget()->objectName();
#endif
		m_blockSignals_bak = m_obj->widget()->blockSignals( true);
	}
	else
	{
		m_blockSignals_bak = true;
	}
}

WidgetVisitor::State::~State()
{
	if (m_blockSignals && !m_blockSignals_bak)
	{
#ifdef WOLFRAME_LOWLEVEL_DEBUG
		qDebug() << "unblock signals of" << m_obj->widget()->metaObject()->className() << m_obj->widget()->objectName();
#endif
		m_obj->widget()->blockSignals( false);
	}
}

QVariant WidgetVisitor::State::dynamicProperty( const QString& name) const
{
	QHash<QString,QVariant>::const_iterator di = m_dynamicProperties.find( name);
	if (di == m_dynamicProperties.end()) return QVariant();
	return di.value();
}

bool WidgetVisitor::State::setDynamicProperty( const QString& name, const QVariant& value, bool allowUndefDynPropsInit)
{
	if (!allowUndefDynPropsInit && !name.startsWith( "_w_") && !name.isEmpty() && !name.at(0).isDigit() && name != "widgetid")
	{
		QHash<QString,QVariant>::const_iterator di = m_dynamicProperties.find( name);
		if (di == m_dynamicProperties.end())
		{
			qWarning() << "set a dynamic property of" << m_obj->widget()->metaObject()->className() << WidgetVisitor::widgetPath( m_obj->widget()) << "that is not predefined:" << name << shortenDebugMessageArgument( value);
		}
	}
	m_dynamicProperties.insert( name, value);
	m_obj->widget()->setProperty( name.toLatin1(), value);
	return true;
}

QString WidgetVisitor::State::getLink( const QString& name) const
{
	int ii = 0, nn = m_links.size();
	for (; ii<nn; ++ii)
	{
		if (m_links.at( ii).first == name)
		{
			return m_links.at( ii).second;
		}
	}
	return QString();
}

void WidgetVisitor::State::defineLink( const QString& name, const QString& value)
{
	int ii = 0, nn = m_links.size();
	for (; ii<nn; ++ii)
	{
		if (m_links.at( ii).first == name)
		{
			m_links[ ii].second = value;
			break;
		}
	}
	if (ii == nn)
	{
		m_links.push_back( LinkDef( name, value));
	}
}

static void getWidgetChildren_( QList<QWidget*>& rt, QObject* wdg)
{
	static const QString str_QWidget("QWidget");
	static const QString str_QStackedWidget("QStackedWidget");
	static const QString str_QTabWidget("QTabWidget");

	foreach (QObject* cld, wdg->children())
	{
		if (qobject_cast<QLayout*>( cld))
		{
			getWidgetChildren_( rt, cld);
		}
		else
		{
			QWidget* we = qobject_cast<QWidget*>( cld);
			if (we)
			{
				QObject* p = we->parent();
				if (we->layout() && cld->metaObject()->className() == str_QWidget
					&& !(p->metaObject()->className() == str_QStackedWidget)
					&& !(p->metaObject()->className() == str_QTabWidget))
				{
					getWidgetChildren_( rt, cld);
				}
				else
				{
					rt.push_back( we);
				}
			}
		}
	}
}

static QList<QWidget*> getWidgetChildren( QWidget* wdg)
{
	QList<QWidget*> rt;
	getWidgetChildren_( rt, wdg);
	return rt;
}

void WidgetVisitor::init_widgetids( QWidget* widget)
{
	setWidgetId( widget);

	foreach (QWidget* child, widget->findChildren<QWidget*>())
	{
		FormWidget* fw = qobject_cast<FormWidget*>(child);
		if (fw) WidgetVisitor::init_widgetids( fw->mainwidget());

		setWidgetId( child);
	}
}

QWidget* WidgetVisitor::get_widget_reference( const QString& id)
{
	if (enter( id, true) && m_stk.top().m_internal_entercnt == 0)
	{
		QWidget* rt = widget();
		leave( true);
		return rt;
	}
	return 0;
}

WidgetVisitor::WidgetVisitor( QWidget* root, VisitorFlags flags_)
	:m_blockSignals(false)
	,m_allowUndefDynPropsInit(false)
{
	if (((int)flags_&(int)BlockSignals) == (int)BlockSignals)
	{
		m_blockSignals = true;
	}
	if (((int)flags_&(int)AllowUndefDynPropsInit) == (int)AllowUndefDynPropsInit)
	{
		m_allowUndefDynPropsInit = true;
	}
	m_stk.push( State( WidgetVisitorObjectR( createWidgetVisitorObject( root)), m_blockSignals));
}

WidgetVisitor::WidgetVisitor( const WidgetVisitorObjectR& obj, VisitorFlags flags_)
	:m_blockSignals(false)
	,m_allowUndefDynPropsInit(false)
{
	if (((int)flags_&(int)BlockSignals) == (int)BlockSignals)
	{
		m_blockSignals = true;
	}
	if (((int)flags_&(int)AllowUndefDynPropsInit) == (int)AllowUndefDynPropsInit)
	{
		m_allowUndefDynPropsInit = true;
	}
	m_stk.push( State( obj, flags_));
}

bool WidgetVisitor::enter( const QString& name, bool writemode)
{
	return enter( name, writemode, 0);
}

bool WidgetVisitor::enter_root( const QString& name)
{
	if (m_stk.empty()) return false;
	if (name == "main")
	{
		FormWidget* wdg = formwidget();
		if (wdg && wdg->mainwidget())
		{
			m_stk.push_back( State( WidgetVisitorObjectR( createWidgetVisitorObject( wdg->mainwidget())), m_blockSignals));
			return true;
		}
	}
	else
	{
		QWidget* ww = predecessor( name);
		if (ww)
		{
			if (ww != m_stk.top().m_obj->widget())
			{
				m_stk.push_back( State( WidgetVisitorObjectR( createWidgetVisitorObject( ww)), m_blockSignals));
				return true;
			}
		}
	}
	return false;
}

QList<QWidget*> WidgetVisitor::children( const QString& name) const
{
	QList<QWidget*> rt;
	if (!m_stk.empty() && !m_stk.top().m_internal_entercnt)
	{
		foreach( QWidget* ww, getWidgetChildren( m_stk.top().m_obj->widget()))
		{
			if (name.isEmpty() || ww->objectName() == name)
			{
				rt.push_back( ww);
			}
		}
	}
	return rt;
}

bool WidgetVisitor::enter( const QString& name, bool writemode, int level)
{
	if (m_stk.empty()) return false;

	// [A] check if name is a multipart reference and follow it if yes:
	int followidx = name.indexOf( '.');
	if (followidx >= 0)
	{
		// ... the referenced item is a multipart reference so it gets complicated
		QString prefix;
		QString rest;
		int entercnt = 0;
		do
		{
			prefix = name.mid( 0, followidx);
			rest = name.mid( followidx+1, name.size()-followidx-1);

			if (!enter( prefix, writemode, level+entercnt))
			{
				for (; entercnt > 0; --entercnt) leave( writemode);
				return false;
			}
			++entercnt;
			followidx = rest.indexOf( '.');
		}
		while (followidx >= 0);

		if (!enter( rest, writemode, level+entercnt))
		{
			for (; entercnt > 0; --entercnt) leave( writemode);
			return false;
		}
		m_stk.top().m_multipart_entercnt = ++entercnt;
		return true;
	}
	// [B] check if name refers to a widget internal item and follow it if yes:
	if (m_stk.top().m_obj->enter( name, writemode))
	{
		TRACE_ENTER( "internal", className(), objectName(), name);
		++m_stk.top().m_internal_entercnt;
		return true;
	}

	if (m_stk.top().m_internal_entercnt == 0)
	{
		// [C] check if name refers to a symbolic link and follow the link if yes:
		QString lnk = m_stk.top().getLink( name);
		if (!lnk.isEmpty())
		{
			QWidget* lnkwdg = resolveLink( resolve( lnk).toString());
			if (!lnkwdg)
			{
				ERROR( "failed to resolve symbolic link to widget (variable ", lnk + ") =>" + resolve( lnk).toString());
				return false;
			}
			m_stk.push_back( State( WidgetVisitorObjectR( createWidgetVisitorObject( lnkwdg)), m_blockSignals));
			TRACE_ENTER( "link", className(), objectName(), resolve(lnk));
			return true;
		}

		// [D] on top level check if name refers to an ancessor or an ancessor child and follow it if yes:
		if (level == 0 && !name.isEmpty() && enter_root( name))
		{
			TRACE_ENTER( "root", className(), objectName(), name);
			return true;
		}

		// [E] check if name refers to a child and follow it if yes:
		if (!name.isEmpty())
		{
			QList<QWidget*> cn = children( name);
			if (cn.size() > 1)
			{
				ERROR( "ambiguos widget reference", name);
				return false;
			}
			if (cn.isEmpty()) return false;
			m_stk.push_back( State( WidgetVisitorObjectR( createWidgetVisitorObject( cn[0])), m_blockSignals));
			TRACE_ENTER( "child", className(), objectName(), name);
			return true;
		}
	}
	return false;
}

void WidgetVisitor::leave( bool writemode)
{
	if (m_stk.empty()) return;
	int cnt = m_stk.top().m_multipart_entercnt;
	while (cnt-- > 0)
	{
		if (m_stk.top().m_internal_entercnt)
		{
			TRACE_LEAVE( "internal")
			if (!m_stk.top().m_obj->leave( writemode))
			{
				ERROR( "illegal state: internal state leave failed");
			}
			--m_stk.top().m_internal_entercnt;
		}
		else
		{
			TRACE_LEAVE( "")
			m_stk.pop();
		}
	}
}

static void append_escaped_( QString& dest, const QString& value)
{
	if (value.indexOf('\'') >= 0 || value.indexOf(',') >= 0 || value.indexOf('&') >= 0)
	{
		dest.push_back( '\'');
		int idx = 0, nxt;
		while ((nxt=value.indexOf( '\'', idx)) >= idx)
		{
			dest.append( value.mid(idx,nxt));
			dest.push_back( '\'');
			dest.push_back( '\'');
			idx = nxt + 1;
		}
		dest.push_back( '\'');
	}
	else
	{
		dest.append( value);
	}
}

static void append_escaped( QString& dest, const QVariant& value)
{
	append_escaped_( dest, value.toString());
}

static QVariant variable_value( WidgetVisitor& visitor, const QString& var)
{
	int dpidx = var.indexOf( ':');
	if (dpidx < 0)
	{
		return visitor.property( var);
	}
	else
	{
		QString value;
		QVariant val = visitor.property( var.mid( 0, dpidx));

		int altidx = var.indexOf( ':', dpidx +1);
		if (altidx >= 0)
		{
			if (val.isValid())
			{
				value = var.mid( dpidx+1, altidx - (dpidx+1)).trimmed();
			}
			else
			{
				value = var.mid( altidx+1, var.size()-(altidx+1)).trimmed();
			}
		}
		else
		{
			if (val.isValid()) return val;
			value = var.mid( dpidx+1, var.size()-(dpidx+1)).trimmed();
		}
		if (value == "?")
		{
			return QVariant();
		}
		foreach (QChar ch, value)
		{
			if (!ch.isDigit() && !ch.isLetter() && ch != '_')
			{
				qCritical() << "illegal character in default value (currently only non negative numbers and alphanumeric or empty value allowed):" << ch << value << "(widget" << visitor.widgetPath() << ")";
				break;
			}
		}
		return QVariant( value);
	}
}

static QVariant expand_variable_references( WidgetVisitor& visitor, const QString& value)
{
	int startidx = 0;
	int substidx = value.indexOf( '{');
	int endidx = value.indexOf( '}', substidx);
	QString rt;
	if (substidx == 0 && endidx == value.size()-1)
	{
		return variable_value( visitor, value.mid( substidx+1, endidx-substidx-1));
	}
	while (substidx >= 0)
	{
		if (endidx < substidx)
		{
			logError( visitor, "brackets { } not balanced", value);
			break;
		}
		rt.append( value.mid( startidx, substidx-startidx));
		int sb = value.indexOf( '{', substidx+1);
		if (sb > 0 && sb < endidx)
		{
			logError( visitor, "brackets { { nested", value);
			break;
		}
		substidx++;

		// evaluate property value and append it expanded as substutute to rt:
		QVariant propvalue = variable_value( visitor, value.mid( substidx, endidx-substidx));
		if (!propvalue.isValid()) return QVariant();

		if (propvalue.type() == QVariant::List)
		{
			QList<QVariant> propvaluelist = propvalue.toList();
			QList<QVariant>::const_iterator li = propvaluelist.begin(), le = propvaluelist.end();
			int lidx = 0;
			for (; li != le; ++li,++lidx)
			{
				if (lidx) rt.push_back( ",");
				append_escaped( rt, *li);
			}
		}
		else
		{
			append_escaped( rt, propvalue.toString());
		}
		startidx = endidx + 1;
		// skip to next embedded variable reference:
		substidx = value.indexOf( '{', startidx);
		endidx = value.indexOf( '}', substidx);
	}
	rt.append( value.mid( startidx, value.size()-startidx));
	return QVariant(rt);
}

QVariant WidgetVisitor::resolve( const QVariant& value)
{
	QString valuestr = value.toString();
	if (valuestr.indexOf( '{') >= 0)
	{
		return expand_variable_references( *this, valuestr);
	}
	return value;
}

static void skipSpaces( QString::const_iterator& xi, const QString::const_iterator& end)
{
	for (; xi != end && xi->isSpace(); ++xi) {}
}

static QVariant getOperand( QString::const_iterator& xi, const QString::const_iterator xe)
{
	QString rt;
	skipSpaces( xi, xe);
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
			return QVariant(rt);
		}
		else
		{
			skipSpaces( xi, xe);
			for (; xi != xe && !xi->isSpace() && *xi != '=' && *xi != '!' && *xi != '>' && *xi != '<'; ++xi)
			{
				rt.push_back( *xi);
			}
			if (rt.isEmpty()) return QVariant();
			return QVariant(rt);
		}
	}
	return QVariant();
}

enum ExprOperator {ExprOpInvalid,ExprOpEq,ExprOpNe,ExprOpGt,ExprOpLt,ExprOpGe,ExprOpLe};
#ifdef WOLFRAME_LOWLEVEL_DEBUG
static const char* exprOperatorName( ExprOperator i)
{
	static const char* ar[] = {"ExprOpInvalid","ExprOpEq","ExprOpNe","ExprOpGt","ExprOpLt","ExprOpGe","ExprOpLe"};
	return ar[i];
}
#endif

static ExprOperator getOperator( QString::const_iterator& xi, const QString::const_iterator xe)
{
	ExprOperator rt = ExprOpInvalid;
	skipSpaces( xi, xe);
	if (xi != xe)
	{
		if (*xi == '>')
		{
			++xi;
			if (xi != xe && *xi == '=')
			{
				rt = ExprOpGe;
				++xi;
			}
			else
			{
				rt = ExprOpGt;
			}
		}
		else if (*xi == '<')
		{
			++xi;
			if (xi != xe && *xi == '=')
			{
				rt = ExprOpLe;
				++xi;
			}
			else if (xi != xe && *xi == '>')
			{
				rt = ExprOpNe;
				++xi;
			}
			else
			{
				rt = ExprOpLt;
			}
		}
		else if (*xi == '!')
		{
			++xi;
			if (xi != xe && *xi == '=')
			{
				rt = ExprOpNe;
				++xi;
			}
		}
		else if (*xi == '=')
		{
			++xi;
			if (xi != xe && *xi == '=')
			{
				++xi;
			}
			rt = ExprOpEq;
		}
	}
	return rt;
}

bool WidgetVisitor::evalCondition( const QVariant& expr)
{
	QList<QString> stack;
	QVariant resolved = resolve(expr);
	if (!resolved.isValid()) return false;
	QString exprstr = resolve(expr).toString().trimmed();
	QString::const_iterator xi = exprstr.begin(), xe = exprstr.end();
	QVariant op1 = getOperand( xi, xe);
	if (xi == xe) return op1.isValid();
	ExprOperator opr = getOperator( xi, xe);
	QVariant op2 = getOperand( xi, xe);
#ifdef WOLFRAME_LOWLEVEL_DEBUG
	qDebug() << "evaluate expression" << op1 << exprOperatorName(opr) << op2;
#endif
	if (!op1.isValid() || !op2.isValid()) return false;
	skipSpaces( xi, xe);
	if (xi != xe)
	{
		qCritical() << "superfluos characters at end of expression" << "(widget" << widgetPath() << ")";
	}
	switch (opr)
	{
		case ExprOpInvalid:
			qCritical() << "invalid operator in conditional expression" << "(widget" << widgetPath() << ")";
			return false;
		case ExprOpEq:
			return op1.toString() == op2.toString();
		case ExprOpNe:
			return op1.toString() != op2.toString();
		case ExprOpGt:
			return op1.toInt() > op2.toInt();
		case ExprOpLt:
			return op1.toInt() < op2.toInt();
		case ExprOpGe:
			return op1.toInt() >= op2.toInt();
		case ExprOpLe:
			return op1.toInt() <= op2.toInt();
	}
	return false;
}

void WidgetVisitor::defineLink( const QString& name, const QString& value)
{
	if (m_stk.isEmpty())
	{
		qCritical() << "internal: illegal call of definedLink" << name << value;
		return;
	}
	else
	{
		m_stk.top().defineLink( name, value);
	}
}

FormWidget* WidgetVisitor::formwidget() const
{
	if (m_stk.isEmpty()) return 0;
	QObject* prn = m_stk.top().m_obj->widget();
	for (; prn != 0; prn = prn->parent())
	{
		FormWidget* fw = qobject_cast<FormWidget*>( prn);
		if (fw) return fw;
	}
	return 0;
}

QWidget* WidgetVisitor::predecessor( const QString& name) const
{
	if (m_stk.isEmpty()) return 0;
	QObject* prn = m_stk.top().m_obj->widget();
	for (; prn != 0; prn = prn->parent())
	{
		QWidget* wdg = qobject_cast<QWidget*>( prn);
		if (wdg)
		{
			if (wdg->objectName() == name) return wdg;
			if (wdg) foreach (QWidget* cld, getWidgetChildren( wdg))
			{
				if (cld->objectName() == name)
				{
					return cld;
				}
			}
		}
		if (qobject_cast<FormWidget*>( wdg)) break;
	}
	return 0;
}

QWidget* WidgetVisitor::uirootwidget() const
{
	if (m_stk.isEmpty()) return 0;
	QWidget* wdg = m_stk.at(0).m_obj->widget();
	QObject* prn = wdg->parent();
	for (; prn != 0; prn = prn->parent())
	{
		if (qobject_cast<QWidget*>( prn))
		{
			wdg = qobject_cast<QWidget*>( prn);
		}
	}
	return wdg;
}

QString WidgetVisitor::widgetPath( const QWidget* widget)
{
	QStringList pt;
	const QObject* prn = widget;
	for (; prn != 0; prn = prn->parent())
	{
		if (qobject_cast<const FormWidget*>( prn)) break;
		if (qobject_cast<const QWidget*>( prn) && !prn->objectName().isEmpty())
		{
			pt.push_back( prn->objectName());
		}
	}
	for(int kk = 0, nn = pt.size(); kk < (nn/2); kk++)
	{
		pt.swap( kk, nn-(1+kk));
	}
	return pt.join( ".");
}

QString WidgetVisitor::widgetPath() const
{
	return widgetPath( widget());
}

QVariant WidgetVisitor::valueAt( const QPoint& p) const
{
	if (m_stk.isEmpty()) return false;
	return m_stk.top().m_obj->valueAt( p);
}

static bool nodeProperty_hasWidgetId( const QWidget* widget, const QVariant& cond)
{
	return widgetIdMatches( cond.toString(), widget);
}

QWidget* WidgetVisitor::resolveLink( const QString& link)
{
	QWidget* wdg = uirootwidget();
	if (!wdg) return 0;
	WidgetVisitor visitor( wdg, None);
	QList<QWidget*> wdglist = visitor.findSubNodes( nodeProperty_hasWidgetId, link);
	if (wdglist.isEmpty()) return 0;
	if (wdglist.size() > 1)
	{
		ERROR( "ambiguus widget link reference", link);
	}
	return wdglist.at(0);
}

FormWidget* WidgetVisitor::findFormWidgetWithWidgetid( const QString& wid)
{
	QWidget* wdg = uirootwidget();
	foreach (FormWidget* fw, wdg->findChildren<FormWidget*>())
	{
		if (widgetIdMatches( wid, fw->mainwidget()))
		{
			return fw;
		}
	}
	return 0;
}

QVariant WidgetVisitor::getState()
{
	QVariant state;
	if (!m_stk.isEmpty())
	{
		state = m_stk.top().m_obj->getState();
	}
	return state;
}

void WidgetVisitor::setState( const QVariant& state)
{
	if (!m_stk.isEmpty())
	{
		m_stk.top().m_obj->setState( state);
	}
}

void WidgetVisitor::endofDataFeed()
{
	if (!m_stk.isEmpty()) m_stk.top().m_obj->endofDataFeed();
}

void WidgetVisitor::clear()
{
	if (!m_stk.isEmpty())
	{
		m_stk.top().m_obj->clear();
	}
}

QVariant WidgetVisitor::property( const QString& name)
{
	return property( name, 0);
}

QVariant WidgetVisitor::property( const char* name)
{
	return property( QString( name), 0);
}

QWidget* WidgetVisitor::getPropertyOwnerWidget( const QString& name) const
{
	if (m_stk.empty()) return 0;
	WidgetVisitor visitor( m_stk.top().m_obj->widget(), flags());
	return visitor.getPropertyOwnerWidget( name, 0);
}

QWidget* WidgetVisitor::getPropertyOwnerWidget( const QString& name, int level)
{
	if (m_stk.empty()) return 0;

	// [A] check if an multipart property is referenced and try to step into the substructure to get the property if yes
	bool subelem = false;
	QString prefix;
	QString rest;
	int followidx = name.indexOf( '.');
	if (followidx >= 0)
	{
		prefix = name.mid( 0, followidx);
		rest = name.mid( followidx+1, name.size()-followidx-1);
		if (enter( prefix, false, level))
		{
			level += 1;
			subelem = true;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		if (enter( name, false, level))
		{
			level += 1;
			subelem = true;
			prefix = name;
			rest.clear();
		}
	}
	if (subelem)
	{
		return getPropertyOwnerWidget( rest, level);
	}
	if (followidx < 0)
	{
		// [B] check if an internal property of the widget is referenced and return its value if yes
		QVariant rt;
		if ((rt = m_stk.top().m_obj->property( name)).isValid())
		{
			TRACE_FETCH( "internal property", objectName(), name, rt)
			return m_stk.top().m_obj->widget();
		}

		// [C] check if a dynamic property is referenced and return its value if yes
		if (m_stk.top().m_internal_entercnt == 0)
		{
			rt = m_stk.top().dynamicProperty( name);
			if (rt.isValid())
			{
				TRACE_FETCH( "dynamic property", objectName(), name, rt)
				return m_stk.top().m_obj->widget();
			}
		}
	}
	return m_stk.top().m_obj->widget();
}

QVariant WidgetVisitor::property( const QString& name, int level)
{
	if (m_stk.empty()) return QVariant()/*invalid*/;

	// [A] check if an multipart property is referenced and try to step into the substructure to get the property if yes
	bool subelem = false;
	QString prefix;
	QString rest;
	int followidx = name.indexOf( '.');
	if (followidx >= 0)
	{
		prefix = name.mid( 0, followidx);
		rest = name.mid( followidx+1, name.size()-followidx-1);
		if (enter( prefix, false, level))
		{
			level += 1;
			subelem = true;
		}
	}
	else
	{
		if (enter( name, false, level))
		{
			level += 1;
			subelem = true;
			prefix = name;
			rest.clear();
		}
	}
	if (subelem)
	{
		QVariant rt = property( rest, level);
		bool isArray = m_stk.top().m_internal_entercnt != 0 && m_stk.top().m_obj->isArrayElement( prefix);
		leave( false);
		if (isArray)
		{
			// ... handle array
			QList<QVariant> rtlist;
			rtlist.push_back( rt);

			while (enter( prefix, false, level))
			{
				rt = property( rest, level);
				leave( false);
				rtlist.push_back( rt);
			}
			return QVariant( rtlist);
		}
		return rt;
	}
	if (followidx < 0)
	{
		// [B] check if an internal property of the widget is referenced and return its value if yes
		QVariant rt;
		if ((rt = m_stk.top().m_obj->property( name)).isValid())
		{
			TRACE_FETCH( "internal property", objectName(), name, rt)
			return resolve( rt);
		}

		// [C] check if a dynamic property is referenced and return its value if yes
		if (m_stk.top().m_internal_entercnt == 0)
		{
			rt = m_stk.top().dynamicProperty( name);
			if (rt.isValid())
			{
				TRACE_FETCH( "dynamic property", objectName(), name, rt)
				return resolve( rt);
			}
		}
	}
	return QVariant();
}


QString WidgetVisitor::objectName() const
{
	if (m_stk.isEmpty()) return QString();
	return m_stk.top().m_obj->widget()->objectName();
}

QString WidgetVisitor::className() const
{
	if (m_stk.isEmpty()) return QString();
	return m_stk.top().m_obj->widget()->metaObject()->className();
}

QString WidgetVisitor::widgetid() const
{
	if (m_stk.isEmpty()) return QString();
	QVariant ruid = getWidgetId( m_stk.top().m_obj->widget());
	if (ruid.type() != QVariant::String)
	{
		ERROR( "property 'widgetid' missing in state");
		return objectName();
	}
	return ruid.toString();
}

bool WidgetVisitor::setProperty( const QString& name, const QVariant& value)
{
	return setProperty( name, value, 0);
}

bool WidgetVisitor::setProperty( const char* name, const QVariant& value)
{
	return setProperty( QString(name), value, 0);
}

bool WidgetVisitor::setProperty( const QString& name, const QVariant& value, int level)
{
	if (m_stk.empty()) return false;

	// [A] check if an multipart property is referenced and try to step into the substructures to set the property (must a single value and must not have any repeating elements) if yes
	bool subelem = false;
	QString prefix;
	QString rest;
	int followidx = name.indexOf( '.');
	if (followidx >= 0)
	{
		prefix = name.mid( 0, followidx);
		rest = name.mid( followidx+1, name.size()-followidx-1);
		TRACE_STATUS( "structured property", name, prefix, rest)
		if (enter( prefix, true, level))
		{
			subelem = true;
		}
		bool isArray = m_stk.top().m_internal_entercnt != 0 && m_stk.top().m_obj->isArrayElement( prefix);
		if (isArray)
		{
			ERROR( "cannot set property addressing a set of properties", prefix);
		}
	}
	else
	{
		TRACE_STATUS( "identifier property", className(), objectName(), name)
		if (enter( name, true, level))
		{
			subelem = true;
			prefix = name;
			rest.clear();
		}
		bool isArray = m_stk.top().m_internal_entercnt != 0 && m_stk.top().m_obj->isArrayElement( name);
		if (isArray)
		{
			ERROR( "cannot set property addressing a set of properties", prefix);
		}
	}
	if (subelem)
	{
		bool rt = setProperty( rest, value, level+1);
		leave( true);
		return rt;
	}
	if (followidx < 0)
	{
		// [B] check if an internal property of the widget is referenced and set its value if yes
		if (m_stk.top().m_obj->setProperty( name, value))
		{
			TRACE_ASSIGNMENT( "internal property", objectName(), name, value)
			return true;
		}

		// [C] check if a dynamic property is referenced and set its value if yes
		if (m_stk.top().m_internal_entercnt == 0)
		{
			TRACE_ASSIGNMENT( "dynamic property", objectName(), name, value)
			if (m_stk.top().setDynamicProperty( name, value, m_allowUndefDynPropsInit)) return true;
		}
	}
	return false;
}

QList<QWidget*> WidgetVisitor::findSubNodes( NodeProperty prop, const QVariant& cond) const
{
	QList<QWidget*> rt;
	if (m_stk.isEmpty()) return rt;
	QVector<QWidget*> ar;

	if (prop( m_stk.top().m_obj->widget(), cond)) rt.push_back( m_stk.top().m_obj->widget());
	ar.push_back( m_stk.top().m_obj->widget());

	int endidx = ar.size(), idx = 0;
	do
	{
		endidx = ar.size();
		while (idx < endidx)
		{
			foreach( QWidget* ww, getWidgetChildren( ar[idx]))
			{
				FormWidget* fw = qobject_cast<FormWidget*>(ww);
				if (fw)
				{
					//PF:HACK: to find form widget
					QWidget* uiw = fw->mainwidget();
					if (prop( uiw, cond)) rt.push_back( uiw);
					ar.push_back( uiw);
					continue;
				}
				if (prop( ww, cond)) rt.push_back( ww);
				ar.push_back( ww);
			}
			++idx;
		}
	} while (endidx < ar.size());
	return rt;
}

static bool nodeProperty_hasGlobal( const QWidget* widget, const QVariant& )
{
	foreach (const QString& prop, widget->dynamicPropertyNames())
	{
		if (prop.startsWith( "global:")) return true;
	}
	return false;
}

void WidgetVisitor::readGlobals( const QHash<QString,QVariant>& globals)
{
	if (m_stk.isEmpty()) return;
	foreach (const State::Assignment& assignment, m_stk.top().m_globals)
	{
		QHash<QString,QVariant>::const_iterator gi = globals.find( assignment.first);
		if (gi != globals.end())
		{
			setProperty( assignment.second, gi.value());
		}
	}
}

void WidgetVisitor::writeGlobals( QHash<QString,QVariant>& globals)
{
	if (m_stk.isEmpty()) return;
	foreach (const State::Assignment& assignment, m_stk.top().m_globals)
	{
		globals[ assignment.first] = property( assignment.second);
	}
}

void WidgetVisitor::do_readGlobals( const QHash<QString,QVariant>& globals)
{
	foreach (QWidget* wdg, findSubNodes( nodeProperty_hasGlobal))
	{
		WidgetVisitor chldvisitor( wdg, flags());
		chldvisitor.readGlobals( globals);
	}
}

void WidgetVisitor::do_writeGlobals( QHash<QString,QVariant>& globals)
{
	foreach (QWidget* wdg, findSubNodes( nodeProperty_hasGlobal))
	{
		WidgetVisitor chldvisitor( wdg, flags());
		chldvisitor.writeGlobals( globals);
	}
}

static bool nodeProperty_hasAssignment( const QWidget* widget, const QVariant& )
{
	foreach (const QString& prop, widget->dynamicPropertyNames())
	{
		if (prop.startsWith( "assign:")) return true;
	}
	return false;
}

void WidgetVisitor::readAssignments()
{
	if (m_stk.isEmpty()) return;
	foreach (const State::Assignment& assignment, m_stk.top().m_assignments)
	{
		QVariant value = property( assignment.second);
		if (!setProperty( assignment.first, value))
		{
			ERROR( "assigment failed", assignment.first);
		}
	}
}

void WidgetVisitor::writeAssignments()
{
	if (m_stk.isEmpty()) return;
	foreach (const State::Assignment& assignment, m_stk.top().m_assignments)
	{
		QVariant value = property( assignment.first);
		if (!setProperty( assignment.second, value))
		{
			ERROR( "assigment failed", assignment.second);
		}
	}
}

void WidgetVisitor::do_readAssignments()
{
	foreach (QWidget* wdg, findSubNodes( nodeProperty_hasAssignment))
	{
		WidgetVisitor chldvisitor( wdg, flags());
		chldvisitor.readAssignments();
	}
}

void WidgetVisitor::do_writeAssignments()
{
	foreach (QWidget* wdg, findSubNodes( nodeProperty_hasAssignment))
	{
		WidgetVisitor chldvisitor( wdg, flags());
		chldvisitor.writeAssignments();
	}
}

bool WidgetVisitor::allowUndefDynPropsInit( bool value)
{
	bool rt = m_allowUndefDynPropsInit;
	m_allowUndefDynPropsInit = value;
	return rt;
}

WidgetListenerImpl* WidgetVisitor::createListener( DataLoader* dataLoader)
{
	WidgetListenerImpl* listener = 0;
	if (!m_stk.isEmpty())
	{
		QWidget* widget = m_stk.top().m_obj->widget();
		listener = new WidgetListenerImpl( widget, dataLoader);
		if (listener)
		{
			if (!m_stk.top().m_datasignals.id[ WidgetListener::SigDestroyed].isEmpty())
			{
				QObject::connect( widget, SIGNAL( destroyed()), listener, SLOT( destroyed()), Qt::UniqueConnection);
			}
			for (int dt=0; dt<WidgetListener::NofDataSignalTypes; ++dt)
			{
				if (!m_stk.top().m_datasignals.id[ dt].isEmpty())
				{
					m_stk.top().m_obj->connectDataSignals( (WidgetListener::DataSignalType)dt, *listener);
				}
			}
		}
	}
	return listener;
}

void WidgetVisitor::connectWidgetEnabler( WidgetEnabler& enabler)
{
	if (!m_stk.isEmpty())
	{
		m_stk.top().m_obj->connectWidgetEnabler( enabler);
	}
}

void WidgetVisitor::ERROR( const char* msg, const QString& arg) const
{
	logError( *this, msg, QString( arg));
}

void WidgetVisitor::ERROR( const char* msg) const
{
	logError( *this, msg);
}

static bool nodeProperty_hasDataSlot( const QWidget* widget, const QVariant& cond)
{
	QVariant dataslots = widget->property( "dataslot");
	int idx = 0;
	while ((idx=dataslots.toString().indexOf( cond.toString(), idx)) >= 0)
	{
		idx += cond.toString().length();
		QString dd = dataslots.toString();
		if (dd.size() == idx || dd.at(idx) == ' ' || dd.at(idx) == ','  || dd.at(idx) == '[') return true;
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

typedef QPair<QString,QWidget*> SignalReceiver;

QList<QPair<QString,QWidget*> > WidgetVisitor::get_datasignal_receivers( const QString& receiverid)
{
	QList<QPair<QString,QWidget*> > rt;
	TRACE_STATUS( "find datasignal receiver", className(), objectName(), receiverid);
	QWidget* rcvwidget;
	QList<QWidget*> wl;
	int atidx = receiverid.indexOf('@');
	QString slotname;
	QString address;
	if (atidx >= 0)
	{
		slotname = receiverid.mid( 0, atidx).trimmed();
		address = receiverid.mid( atidx+1, receiverid.size() - (atidx+1)).trimmed();
	}
	else
	{
		address = receiverid;
	}
	if (WidgetId::isValid( address))
	{
		WidgetVisitor mainvisitor( uirootwidget(), None);
		wl.append( mainvisitor.findSubNodes( nodeProperty_hasWidgetId, address));
		foreach (QWidget* rcvwidget, wl)
		{
			TRACE_STATUS( "found widget by address", rcvwidget->metaObject()->className(), rcvwidget->objectName(), address);
			rt.push_back( SignalReceiver( slotname, rcvwidget));
		}
	}
	else if (address.indexOf('.') >= 0)
	{
		rcvwidget = get_widget_reference( address);
		if (rcvwidget)
		{
			TRACE_STATUS( "found widget reference", rcvwidget->metaObject()->className(), rcvwidget->objectName(), address);
			rt.push_back( SignalReceiver( slotname, rcvwidget));
		}
		else
		{
			ERROR( "unable to resolve local widget reference:", address);
		}
	}
	else if ((rcvwidget = get_widget_reference( address)) != 0)
	{
		TRACE_STATUS( "found widget reference", rcvwidget->metaObject()->className(), rcvwidget->objectName(), address);
		rt.push_back( SignalReceiver( slotname, rcvwidget));
	}
	else
	{
		if (slotname.isEmpty())
		{
			slotname = address;
		}
		WidgetVisitor mainvisitor( uirootwidget(), None);
		QWidget* thiswidget = widget();
		foreach (QWidget* rcvwidget, mainvisitor.findSubNodes( nodeProperty_hasDataSlot, address))
		{
			if (rcvwidget != thiswidget)
			{
				QVariant sendercond = getDatasignalSender( rcvwidget, address);
				if (sendercond.isValid())
				{
					if (sendercond.toString() == widgetid())
					{
						TRACE_STATUS( "found widget by data slot identifier with sender id", rcvwidget->metaObject()->className(), rcvwidget->objectName(), getWidgetId(rcvwidget));
						rt.push_back( SignalReceiver( slotname, rcvwidget));
					}
				}
				else
				{
					TRACE_STATUS( "found widget by data slot identifier", rcvwidget->metaObject()->className(), rcvwidget->objectName(), getWidgetId(rcvwidget));
					rt.push_back( SignalReceiver( slotname, rcvwidget));
				}
			}
		}
	}
	return rt;
}

QList<QPair<QString,QWidget*> > WidgetVisitor::get_datasignal_receivers( WidgetListener::DataSignalType type)
{
	QList<QPair<QString,QWidget*> > rt;
	if (m_stk.isEmpty()) return rt;

	foreach (const QString& receiverprop, m_stk.top().m_datasignals.id[(int)type])
	{
		TRACE_STATUS( "resolve data signal receiver", className(), objectName(), receiverprop);
		QVariant receiverid = resolve( receiverprop);
		rt.append( get_datasignal_receivers( receiverid.toString()));
	}
	return rt;
}



