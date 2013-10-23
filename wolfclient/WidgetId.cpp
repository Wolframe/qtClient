#include "WidgetId.hpp"

static qint64 g_cnt = 0;

QVariant getWidgetId( const QWidget* wdg)
{
	return wdg->property( "widgetid");
}

QString askWidgetId( QWidget* wdg)
{
	QVariant rt = wdg->property( "widgetid");
	if (rt.isValid()) return rt.toString();

	QString newid =  wdg->objectName();
	newid.append( ":");
	newid.append( QVariant( ++g_cnt).toString());
	wdg->setProperty( "widgetid", newid);
	return newid;
}

void setWidgetId( QWidget* wdg)
{
	(void)askWidgetId( wdg);
}

bool widgetIdMatches( const QString& id, const QWidget* wdg)
{
	if (id.startsWith(wdg->objectName()) && wdg->objectName().size() < id.size() && id.at(wdg->objectName().size()) == ':')
	{
		QVariant wid = getWidgetId( wdg);
		return (wid.toString() == id);
	}
	return false;
}

bool isWidgetId( const QString& id)
{
	return id.indexOf(':') >= 0;
}

