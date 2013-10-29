#include "WidgetRequestHeader.hpp"
#include "WidgetId.hpp"

WidgetRequestHeader::WidgetRequestHeader( QWidget* recipient_widgetid)
{
	recipient.widgetid = askWidgetId( recipient_widgetid);
}

WidgetRequestHeader::WidgetRequestHeader( const QString& str)
{
	type = getType( str);
	actionid = getActionId( str);
	followform = getFollowForm( str);
	command = getCommand( str);
	recipient.widgetid = getWidgetId( str);
}

WidgetRequestHeader::WidgetRequestHeader( const WidgetRequestHeader& o)
	:actionid(o.actionid)
	,followform(o.followform)
	,command(o.command)
{
	recipient.widgetid = o.recipient.widgetid;
}

QString WidgetRequestHeader::toLogIdString() const
{
	QString rt;
	switch (type)
	{
		case Undefined: rt.append( "undefined"); break;
		case Load: rt.append( "dataload"); break;
		case Action: rt.append( "action"); break;
	}
	if (actionid.isValid())
	{
		rt.append( ".");
		rt.append( actionid.toString());
	}
	rt.append( "(");
	if (recipient.widgetid.isValid())
	{
		rt.append( recipient.widgetid.toString());
	}
	rt.append( ")");
	return rt;
}

QString WidgetRequestHeader::toString() const
{
	QString rt;
	switch (type)
	{
		case Undefined: break;
		case Load: rt.append( "t=L:"); break;
		case Action: rt.append( "t=A:"); break;
	}
	if (actionid.isValid())
	{
		rt.append( "a=");
		rt.append( actionid.toString());
		rt.append( ":");
	}
	if (command.isValid())
	{
		rt.append( "c=");
		rt.append( command.toString());
		rt.append( ":");
	}
	if (followform.isValid())
	{
		rt.append( "f=");
		rt.append( followform.toString());
		rt.append( ":");
	}
	if (recipient.widgetid.isValid())
	{
		rt.append( "w=");
		rt.append( recipient.widgetid.toString());
	}
	return rt;
}

QVariant WidgetRequestHeader::getWidgetId( const QString& hdrstr)
{
	int idx = hdrstr.indexOf("w=");
	if (idx >= 0) return hdrstr.mid( idx+2);
	return QVariant();
}

static QVariant getId( const QString& hdrstr, const char* id)
{
	int idx = hdrstr.indexOf( id);
	if (idx >= 0)
	{
		idx += 2;
		int end = hdrstr.indexOf( ':', idx);
		if (end >= 0)
		{
			return hdrstr.mid( idx, end-idx);
		}
		else
		{
			return hdrstr.mid( idx);
		}
	}
	return QVariant();
}

QVariant WidgetRequestHeader::getActionId( const QString& hdrstr)
{
	return getId( hdrstr, "a=");
}

QVariant WidgetRequestHeader::getFollowForm( const QString& hdrstr)
{
	return getId( hdrstr, "f=");
}

QVariant WidgetRequestHeader::getCommand( const QString& hdrstr)
{
	return getId( hdrstr, "c=");
}

WidgetRequestHeader::Type WidgetRequestHeader::getType( const QString& hdrstr)
{
	QString typenam = getId( hdrstr, "t=").toString();
	if (typenam == "L") return Load;
	if (typenam == "A") return Action;
	return Undefined;
}

