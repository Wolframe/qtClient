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
#include <QVariant>
#define WOLFRAME_LOWLEVEL_DEBUG
#ifdef WOLFRAME_LOWLEVEL_DEBUG
#define TRACE_VALUE( TITLE, VALUE)			qDebug() << "widget answer XML " << (TITLE) << (VALUE);
#define TRACE_ASSIGNMENT( TITLE, NAME, VALUE)		qDebug() << "widget answer XML " << (TITLE) << (NAME) << "=" << (VALUE);
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
	bool useSynonymsValue = visitor.useSynonyms( false);

	QWidget* widget = visitor.widget();
	QString actionstr( actiondef.toString());

	ActionDefinition action( actionstr);
	qDebug() << "checking action condition properties" << action.condProperties();
	foreach (const QString& cond, action.condProperties())
	{
		if (!visitor.property( cond).isValid())
		{
			// one of the preconditions is not met, return empty (no) request
			qDebug() << "suppressing action" << actionstr << "because condition" << cond << "is not met (condition not valid)";
			visitor.useSynonyms( useSynonymsValue);
			return rt;
		}
	}
	QString docType = action.doctype();
	QString rootElement = action.rootelement();
	bool isStandalone = rootElement.isEmpty();
	if (!action.isValid())
	{
		qCritical() << "invalid request for action doctype=" << docType << "root=" << rootElement;
		visitor.useSynonyms( useSynonymsValue);
		return rt;
	}
	QList<DataSerializeItem> elements = getWidgetDataSerialization( action.structure(), widget);
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
		qDebug() << "action request of " << visitor.objectName() << "=" << rt.tag << ":" << rt.content;
	}
	else
	{
		rt.tag = WidgetRequest::actionWidgetRequestTag( visitor.widgetid(), menuitem);
		qDebug() << "menu" << menuitem << "action request of " << visitor.objectName() << "=" << rt.tag << ":" << rt.content;
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

WidgetRequest getWidgetRequest( WidgetVisitor& visitor, const QString& actiondef, bool debugmode)
{
	WidgetRequest rt = getWidgetRequest_( visitor, actiondef, debugmode);
	rt.tag = WidgetRequest::domainLoadWidgetRequestTag( visitor.widgetid());
	qDebug() << "widget request [" << actiondef << "] of " << visitor.objectName() << "=" << rt.tag << ":" << rt.content;
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

static void XMLERROR( const QList<WidgetAnswerStackElement>& stk, const QString& message)
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

bool setWidgetAnswer( WidgetVisitor& visitor, const QByteArray& answer)
{
	QList<WidgetAnswerStackElement> stk;
	QList<DataSerializeItem> itemlist = getXMLSerialization( "", "", answer);

	visitor.clear();
	qDebug() << "feeding widget " << visitor.objectName() << "with XML";

	DataSerializeItem::Type prevType = DataSerializeItem::CloseTag;
	QString attributename;

	foreach( const DataSerializeItem& item, itemlist)
	{
		TRACE_ASSIGNMENT( "answer element", DataSerializeItem::typeName( item.type()), item.value())
	}
	foreach( const DataSerializeItem& item, itemlist)
	{
		switch (item.type())
		{
			case DataSerializeItem::OpenTag:
			{
				TRACE_VALUE( "OPEN TAG", item.value());
				if (!stk.isEmpty() && !stk.last().istag)
				{
					XMLERROR( stk, QString( "element not defined: '") + stk.last().name + "/" + item.value().toString() + "'");
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
					XMLERROR( stk, QString( "unexpected end element: XML tags not balanced"));
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
					TRACE_ASSIGNMENT( "ATTRIBUTE", attributename, item.value());
					if (!visitor.setProperty( attributename, item.value()))
					{
						XMLERROR( stk, QString( "failed to set property '") + attributename + "'");
					}
				}
				else if (stk.last().istag)
				{
					TRACE_VALUE( "CONTENT", item.value());
					if (!visitor.setProperty( "", item.value()))
					{
						XMLERROR( stk, "failed to set content element");
					}
				}
				else
				{
					TRACE_ASSIGNMENT( "PROPERTY", stk.last().name, item.value());
					if (!visitor.setProperty( stk.last().name, item.value()))
					{
						XMLERROR( stk, QString( "failed to set property '") + stk.last().name + "'");
					}
				}
			break;
		}
		prevType = item.type();
	}
	visitor.endofDataFeed();
	return true;
}





