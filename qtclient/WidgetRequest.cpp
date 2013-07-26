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
#include "WidgetRequest.hpp"
#include "WidgetVisitor.hpp"
#include "DebugTerminal.hpp"
#include "DataSerializeItem.hpp"
#include "DataTreeSerialize.hpp"
#include "DataFormatXML.hpp"
#include "DebugHelpers.hpp"
#include <QVariant>

#undef WOLFRAME_LOWLEVEL_DEBUG
#ifdef WOLFRAME_LOWLEVEL_DEBUG
#define TRACE_VALUE( TITLE, VALUE)			qDebug() << "[widget request]" << (TITLE) << shortenDebugMessageArgument(QVariant(VALUE));
#define TRACE_ASSIGNMENT( TITLE, NAME, VALUE)		qDebug() << "[widget request]" << (TITLE) << (NAME) << "=" << shortenDebugMessageArgument(QVariant(VALUE));
#else
#define TRACE_VALUE( TITLE, VALUE)
#define TRACE_ASSIGNMENT( TITLE, NAME, VALUE)
#endif

QString WidgetRequest::actionWidgetRequestTag( QString recipientid_)
{
	return QString("-") + recipientid_;
}

QString WidgetRequest::actionWidgetRequestTag( QString recipientid_, QString followform_)
{
	return QString("-") + recipientid_ + "-" + followform_;
}

QString WidgetRequest::domainLoadWidgetRequestTag( QString recipientid_)
{
	return recipientid_;
}

WidgetRequest::Type WidgetRequest::type() const
{
	if (tag.size() && tag.at(0) == '-') return Action;
	return DomainLoad;
}

QString WidgetRequest::recipientid() const
{
	if (type() == Action)
	{
		int formidx = tag.indexOf( '-', 1);
		if (formidx < 0)
		{
			return tag.mid( 1, tag.size()-1);
		}
		else
		{
			return tag.mid( 1, formidx-1);
		}
	}
	return tag;
}

QString WidgetRequest::followform() const
{
	int formidx = tag.indexOf( '-', 1);
	if (formidx < 0)
	{
		return "";
	}
	else
	{
		return tag.mid( formidx+1, tag.size()-formidx-1);
	}
}

static WidgetRequest getWidgetRequest_( WidgetVisitor& visitor, const QVariant& actiondef, bool debugmode)
{
	WidgetRequest rt;

	QString actionstr( actiondef.toString());

	ActionDefinition action( actionstr);
	foreach (const QString& cond, action.condProperties())
	{
		if (!visitor.property( cond).isValid())
		{
			// one of the preconditions is not met, return empty (no) request
			qDebug() << "suppressing action" << actionstr << "because condition" << cond << "is not met (condition not valid)";
			return rt;
		}
	}
	QString docType = action.doctype();
	QString rootElement = action.rootelement();
	bool isStandalone = rootElement.isEmpty();
	if (!action.isValid())
	{
		qCritical() << "invalid request for action doctype=" << docType << "root=" << rootElement;
		return rt;
	}
	QList<DataSerializeItem> elements = getWidgetDataSerialization( action.structure(), visitor);
#ifdef WOLFRAME_LOWLEVEL_DEBUG
	foreach (const DataSerializeItem& ei, elements)
	{
		TRACE_VALUE( "serialized data element", ei.toString())
	}
#endif
	rt.cmd = action.command();
	rt.content = getDataXML( docType, rootElement, isStandalone, elements, debugmode);
	return rt;
}

WidgetRequest getActionRequest( WidgetVisitor& visitor, const QVariant& actiondef, bool debugmode, const QString& menuitem)
{
	WidgetRequest rt;
	QWidget* widget = visitor.widget();
	if (!widget)
	{
		qCritical() << "invalid request (no widget defined)";
		return rt;
	}
	rt = getWidgetRequest_( visitor, actiondef, debugmode);
	if (menuitem.isEmpty())
	{
		rt.tag = WidgetRequest::actionWidgetRequestTag( visitor.widgetid());
		qDebug() << "action request of " << visitor.objectName() << "=" << rt.tag << ":" << shortenDebugMessageArgument( rt.content );
	}
	else
	{
		rt.tag = WidgetRequest::actionWidgetRequestTag( visitor.widgetid(), menuitem);
		qDebug() << "menu" << menuitem << "action request of " << visitor.objectName() << "=" << rt.tag << ":" << shortenDebugMessageArgument( rt.content );
	}
	return rt;
}

QList<QString> getMenuActionRequestProperties( WidgetVisitor& visitor, const QString& menuitem)
{
	QWidget* widget = visitor.widget();
	QByteArray propname = QByteArray("action:") + menuitem.toLatin1();
	QVariant action = widget->property( propname);
	if (!action.isValid()) return QList<QString>();
	ActionDefinition actiondef( action.toString());
	return actiondef.condProperties();
}

QList<QString> getActionRequestProperties( WidgetVisitor& visitor)
{
	QWidget* widget = visitor.widget();
	QVariant action = widget->property( "action");
	if (!action.isValid()) return QList<QString>();
	ActionDefinition actiondef( action.toString());
	return actiondef.condProperties();
}

WidgetRequest getWidgetRequest( WidgetVisitor& visitor, bool debugmode)
{
	WidgetRequest rt;
	QWidget* widget = visitor.widget();
	if (!widget)
	{
		qCritical() << "request on non existing widget";
		return rt;
	}
	QVariant action_v = widget->property( "action");
	if (!action_v.isValid())
	{
		qCritical() << "undefined request. action (property action) does not exist in" << visitor.className() << visitor.objectName();
		return rt;
	}
	rt = getWidgetRequest_( visitor, action_v, debugmode);
	rt.tag = WidgetRequest::domainLoadWidgetRequestTag( visitor.widgetid());
	qDebug() << "widget request of " << visitor.objectName() << "=" << rt.tag << ":" << rt.content;
	return rt;
}

WidgetRequest getWidgetRequest( WidgetVisitor& visitor, const QString& actiondef, bool debugmode, const QString& menuitem)
{
	WidgetRequest rt = getWidgetRequest_( visitor, actiondef, debugmode);
	rt.tag = WidgetRequest::domainLoadWidgetRequestTag( visitor.widgetid());
	qDebug() << "widget request [" << menuitem << "] of " << visitor.objectName() << "=" << rt.tag << ":" << rt.content;
	return rt;
}

struct WidgetAnswerStackElement
{
	QString name;
	QString tok;
	bool istag;
	bool ischild;

	explicit WidgetAnswerStackElement( const QString& name_, bool istag_, bool ischild_)
		:name(name_)
		,tok()
		,istag(istag_)
		,ischild(ischild_)
	{}

	WidgetAnswerStackElement( const WidgetAnswerStackElement& o)
		:name( o.name)
		,tok( o.tok)
		,istag(o.istag)
		,ischild(o.ischild)
	{}
};

static void logError( const QList<WidgetAnswerStackElement>& stk, const QString& message)
{
	QString path;
	QList<WidgetAnswerStackElement>::const_iterator pi = stk.begin(), pe = stk.end();
	for (; pi != pe; ++pi)
	{
		path.append( "/");
		path.append( pi->name);
	}
	qCritical() << (message.isEmpty()?QString("error"):message) << "in XML tag" << path;
}


typedef QPair<QString,QString> RewriteRule;
static QList<DataSerializeItem> rewriteSerialization( const QList<RewriteRule>& rewriteRules, const QList<DataSerializeItem>& itemlist)
{
	QList<DataSerializeItem> tempres( itemlist);
	QList<DataSerializeItem> rt;
	QList<RewriteRule>::const_iterator ri = rewriteRules.begin(), re = rewriteRules.end();
	for (; ri != re; ++ri)
	{
		if (ri->first.isEmpty())
		{
			qCritical() << "rewrite rules with empty key element are not supported";
		}
		else if (ri->first.indexOf( '.') >= 0)
		{
			qCritical() << "rewrite rules with more than one key element are not supported:" << ri->first;
			continue;
		}
		QList<DataSerializeItem>::iterator di = tempres.begin(), de = tempres.end();
		for (; di != de; ++di)
		{
			if (di->type() == DataSerializeItem::OpenTag || di->type() == DataSerializeItem::Attribute)
			{
				if (di->value().toString() == ri->first)
				{
					if (ri->second.trimmed() == "?")
					{
						TRACE_VALUE( "DELETE", ri->first);
						di->value() = QVariant();
					}
					else
					{
						TRACE_ASSIGNMENT( "REWRITE", ri->first, ri->second);
						di->value() = QVariant( ri->second);
					}
				}
			}
		}
	}
	QList<int> closecnt_stk;
	QList<DataSerializeItem>::iterator di = tempres.begin(), de = tempres.end();
	for (; di != de; ++di)
	{
		if (di->type() == DataSerializeItem::Attribute)
		{
			if (di->value().isValid())
			{
				QList<QString> tags = di->value().toString().trimmed().split('.');
				int ii=1, nn = tags.size();
				for (; ii<nn; ++ii)
				{
					rt.push_back( DataSerializeItem( DataSerializeItem::OpenTag, tags.at(ii-1).trimmed()));
				}
				rt.push_back( DataSerializeItem( DataSerializeItem::Attribute, tags.back()));
				++di;
				if (di->type() != DataSerializeItem::Value)
				{
					qCritical() << "invalid data tree serialization (value expected after attribute)";
					break;
				}
				rt.push_back( *di);
				for (ii=1; ii<nn; ++ii)
				{
					rt.push_back( DataSerializeItem( DataSerializeItem::CloseTag, QVariant()));
				}
			}
			else
			{
				++di;
				if (di->type() != DataSerializeItem::Value)
				{
					qCritical() << "invalid data tree serialization (value expected after attribute)";
					break;
				}
			}
		}
		else if (di->type() == DataSerializeItem::CloseTag)
		{
			if (closecnt_stk.isEmpty())
			{
				qCritical() << "tags not balanced (unexpected close tag)";
				break;
			}
			int ii = 0, nn = closecnt_stk.back();
			for (; ii<nn; ++ii)
			{
				rt.push_back( DataSerializeItem( DataSerializeItem::CloseTag, QVariant()));
			}
			closecnt_stk.pop_back();
		}
		else if (di->type() == DataSerializeItem::OpenTag)
		{
			if (di->value().isValid())
			{
				QList<QString> tags = di->value().toString().trimmed().split('.');
				int ii = 0, nn = tags.size();
				for (; ii<nn; ++ii)
				{
					rt.push_back( DataSerializeItem( DataSerializeItem::OpenTag, tags.at(ii).trimmed()));
				}
				closecnt_stk.push_back( nn);
			}
			else
			{
				int taglevel = 1;
				for (++di; di != de; ++di)
				{
					if (di->type() == DataSerializeItem::OpenTag)
					{
						++taglevel;
					}
					else if (di->type() == DataSerializeItem::CloseTag)
					{
						--taglevel;
						if (taglevel == 0) break;
					}
				}
				if (di == de)
				{
					qCritical() << "tags not balanced (missing close tag)";
					break;
				}
			}
		}
		else
		{
			rt.push_back( *di);
		}
	}
	return rt;
}

static bool setImplicitWidgetAnswer( WidgetVisitor& visitor, const QByteArray& answer)
{
	QList<WidgetAnswerStackElement> stk;
	QList<DataSerializeItem> itemlist = getXMLSerialization( "", "", answer);

	visitor.clear();
	qDebug() << "feeding widget " << visitor.objectName() << "with implicitely mapped answer";

	DataSerializeItem::Type prevType = DataSerializeItem::CloseTag;
	QString attributename;

	QList<RewriteRule> rewriteRules;
	if (visitor.widget()) foreach (const QByteArray& prop, visitor.widget()->dynamicPropertyNames())
	{
		if (prop.indexOf(':') >= 0)
		{
			if (prop.startsWith( "synonym:"))
			{
				QString name = prop.mid( 8, prop.size()-8);
				QVariant synonym = visitor.property( (const char*)prop);
				rewriteRules.push_back( RewriteRule( name, synonym.toString()));
				qDebug() << "found rewrite rule:" << name << "->" << synonym.toString();
			}
		}
	}
	if (!rewriteRules.isEmpty())
	{
		itemlist = rewriteSerialization( rewriteRules, itemlist);
	}
	foreach( const DataSerializeItem& item, itemlist)
	{
		TRACE_ASSIGNMENT( "answer element", DataSerializeItem::typeName( item.type()), item.value())

		switch (item.type())
		{
			case DataSerializeItem::OpenTag:
			{
				TRACE_VALUE( "OPEN TAG", item.value());
				if (!stk.isEmpty() && !stk.last().istag)
				{
					logError( stk, QString( "element not defined: '") + stk.last().name + "/" + item.value().toString() + "'");
					return false;
				}
				QWidget* prev_widget = visitor.widget();
				bool istag = visitor.enter( item.value().toString(), true);
				bool ischild = (prev_widget != visitor.widget());
				stk.push_back( WidgetAnswerStackElement( item.value().toString(), istag, ischild));
			}
			break;
			case DataSerializeItem::CloseTag:
				TRACE_VALUE( "CLOSE TAG", "");
				if (stk.isEmpty())
				{
					logError( stk, QString( "unexpected end element: XML tags not balanced"));
					return false;
				}
				if (stk.last().ischild)
				{
					visitor.endofDataFeed();
				}
				if (stk.last().istag)
				{
					visitor.leave( true);
				}
				stk.removeLast();
			break;
			case DataSerializeItem::Attribute:
				attributename = item.value().toString();
			break;
			case DataSerializeItem::Value:
				if (prevType == DataSerializeItem::Attribute)
				{
					if (item.value().isValid())
					{
						TRACE_ASSIGNMENT( "ATTRIBUTE", attributename, item.value());
						if (!visitor.setProperty( attributename, item.value()))
						{
							logError( stk, QString( "failed to set property (attribute)'") + attributename + "'");
						}
					}
					else
					{
						TRACE_VALUE( "SKIP STATEMENT TO", attributename);
					}
				}
				else if (stk.last().istag)
				{
					if (item.value().isValid())
					{
						TRACE_VALUE( "CONTENT", item.value());
						if (!visitor.setProperty( "", item.value()))
						{
							logError( stk, "failed to set content element");
						}
					}
					else
					{
						TRACE_VALUE( "SKIP STATEMENT TO", "(content)");
					}
				}
				else
				{
					if (item.value().isValid())
					{
						TRACE_ASSIGNMENT( "PROPERTY", stk.last().name, item.value());
						if (!visitor.setProperty( stk.last().name, item.value()))
						{
							logError( stk, QString( "failed to set property (content value)'") + stk.last().name + "'");
						}
					}
					else
					{
						TRACE_VALUE( "SKIP STATEMENT TO", stk.last().name);
					}
				}
			break;
		}
		prevType = item.type();
	}
	visitor.endofDataFeed();
	return true;
}

struct AssignIterStackElem
{
	typedef QList<WidgetDataAssignmentInstr>::const_iterator AssignIter;
	AssignIter iter;
	int arraypos;

	AssignIterStackElem()					:arraypos(0){}
	AssignIterStackElem( AssignIter iter_)			:iter(iter_),arraypos(0){}
	AssignIterStackElem( const AssignIterStackElem& o)	:iter(o.iter),arraypos(o.arraypos){}
};

static const char* getPropertyAccessErrorReason( WidgetVisitor& visitor, const QString& prop)
{
	if (!visitor.getPropertyOwnerWidget( prop))
	{
		return "owner widget undefined (invalid widget path)";
	}
	else
	{
		return "accessor function failed";
	}
}

static bool setValidatedWidgetAnswer( WidgetVisitor& visitor, const QString& resultschema, const QByteArray& answer)
{
	bool rt = true;
	ActionResultDefinition resultdef( resultschema);
#ifdef WOLFRAME_LOWLEVEL_DEBUG
	qDebug( ) << "answer (as string): " << resultschema;
	qDebug( ) << "answer (structure, tostring): " << resultdef.toString( );	
#endif

	QList<DataSerializeItem> itemlist = getXMLSerialization( resultdef.doctype(), resultdef.rootelement(), answer);
#ifdef WOLFRAME_LOWLEVEL_DEBUG
	foreach( DataSerializeItem item, itemlist ) {
		qDebug( ) << "data serialization structure: " << item.toString( );
	}
#endif
	
	visitor.clear();
	qDebug() << "feeding widget " << visitor.objectName() << "with validated answer";

	QList<WidgetDataAssignmentInstr> assignments = getWidgetDataAssignments( resultdef.structure(), itemlist);
#ifdef WOLFRAME_LOWLEVEL_DEBUG
	foreach( WidgetDataAssignmentInstr instr, assignments ) {
		qDebug( ) << "widget data assign instr: " << instr.toString( );
	}
#endif

	QList<AssignIterStackElem> astk;

	QList<WidgetDataAssignmentInstr>::const_iterator ai = assignments.begin(), ae = assignments.end();
	QList<int> aidxposar;
	for (; ai != ae; ++ai)
	{
		aidxposar.push_back(0);
		TRACE_ASSIGNMENT( QString("answer assignment ") + WidgetDataAssignmentInstr::typeName( ai->type), ai->name, shortenDebugMessageArgument( ai->value));
	}
	for (ai = assignments.begin(); ai != ae; ++ai)
	{
		switch (ai->type)
		{
			case WidgetDataAssignmentInstr::Enter:
				if (ai->arraysize > 0)
				{
					TRACE_VALUE( "enter", ai->name)
					if (!visitor.enter( ai->name, true))
					{
						qCritical() << "failed to find widget substructure" << ai->name << "(explanation:" << getPropertyAccessErrorReason( visitor, ai->name) << ")";
						return false;
					}
					astk.push_back( AssignIterStackElem( ai));
				}
				else
				{
					qDebug() << "widget assignment skiping empty array" << ai->name;
					//...if array is empty then skip it
					int taglevel = 0;
					QList<WidgetDataAssignmentInstr>::const_iterator ac = ai;
					for (; ac != ae; ++ac)
					{
						if (ac->type == WidgetDataAssignmentInstr::Enter)
						{
							++taglevel;
						}
						else if (ac->type == WidgetDataAssignmentInstr::Leave)
						{
							--taglevel;
						}
						if (taglevel == 0)
						{
							ai = ac;
							break;
						}
					}
					if (ac == ae) break;
				}
				break;

			case WidgetDataAssignmentInstr::Leave:
				TRACE_VALUE( "leave", ai->name)
				visitor.leave( true);
				if (astk.back().arraypos+1 < astk.back().iter->arraysize)
				{
					TRACE_VALUE( "enter (again)", astk.back().iter->name)
					visitor.enter( astk.back().iter->name, true);
					astk.back().arraypos++;
					ai = astk.back().iter;
				}
				else
				{
					astk.pop_back();
				}
				break;

			case WidgetDataAssignmentInstr::Assign:
				if (ai->value.type() == QVariant::List)
				{
					if (astk.isEmpty())
					{
						TRACE_ASSIGNMENT( "set property", ai->name, ai->value)
						if (!visitor.setProperty( ai->name, ai->value))
						{
							qCritical() << "failed to set property (list assignment)" << ai->name  << "(explanation:" << getPropertyAccessErrorReason( visitor, ai->name) << ")";
							rt = false;
						}
					}
					else
					{
						int apos = aidxposar.at( ai-assignments.begin());
						QVariant aelem = ai->value.toList().at( apos);
						if (aelem.isValid())
						{
							TRACE_ASSIGNMENT( "set property", ai->name, aelem)
							if (!visitor.setProperty( ai->name, aelem))
							{
								qCritical() << "failed to set property (list element assignment in array)" << ai->name << "[" << apos << "] value=" << shortenDebugMessageArgument(aelem)  << "(explanation:" << getPropertyAccessErrorReason( visitor, ai->name) << ")";
								rt = false;
							}
						}
						else
						{
							TRACE_VALUE( "skip set property (undefined value)", ai->name)
						}
						++aidxposar[ ai-assignments.begin()];
					}
				}
				else
				{
					if (!astk.isEmpty())
					{
						int apos = aidxposar.at( ai-assignments.begin());
						if (apos)
						{
							qCritical() << "referencing element out of range" << ai->name << "(" << apos << ")";
							break;
						}
						++aidxposar[ ai-assignments.begin()];
					}
					TRACE_ASSIGNMENT( "set property", ai->name, ai->value)
					if (!visitor.setProperty( ai->name, ai->value))
					{
						qCritical() << "failed to set property (single element)" << ai->name << ai->value << "(explanation:" << getPropertyAccessErrorReason( visitor, ai->name) << ")";
						rt = false;
					}
				}
				break;
		}
	}
	return rt;
}

bool setWidgetAnswer( WidgetVisitor& visitor, const QByteArray& answer)
{
	QWidget* widget = visitor.widget();
	QVariant resultschema = widget->property( "answer");

	if (resultschema.isValid())
	{
		return setValidatedWidgetAnswer( visitor, resultschema.toString(), answer);
	}
	else
	{
		return setImplicitWidgetAnswer( visitor, answer);
	}
}




