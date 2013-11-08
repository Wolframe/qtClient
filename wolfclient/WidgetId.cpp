#include "WidgetId.hpp"
#include <QDebug>

Q_DECLARE_METATYPE(WidgetId)

static qint64 g_cnt = 0;
static int g_metatype_id = qRegisterMetaType<WidgetId>( WIDGETID_MIMETYPE);

QVariant getWidgetId( const QWidget* wdg)
{
	return wdg->property( "widgetid");
}

QString askWidgetId( QWidget* wdg)
{
	QVariant rt = wdg->property( "widgetid");
	if (rt.isValid()) return rt.toString();

	QString newid =  wdg->objectName();
	if (newid.size() && !newid.startsWith("qt_"))
	{
		newid.append( ":");
		newid.append( QVariant( ++g_cnt).toString());
		wdg->setProperty( "widgetid", newid);
	}
	return newid;
}

void setWidgetId( QWidget* wdg)
{
	(void)askWidgetId( wdg);
}

bool widgetIdMatches( const QString& id, const QWidget* wdg)
{
	QVariant wid = getWidgetId( wdg);
	return (wid.isValid() && wid.toString() == id);
}

bool isWidgetId( const QString& id)
{
	return id.indexOf(':') >= 0;
}

QString getObjectNameFromWidgetId( const QString& wid)
{
	int endpos = wid.indexOf(':');
	if (endpos < 0) endpos = wid.size();
	return wid.mid( 0, endpos);
}

int getCntWidgetId( const QString& wid)
{
	int startpos = wid.indexOf(':');
	if (startpos < 0) return -1;
	return wid.mid( startpos).toInt();
}

WidgetId::WidgetId( const QWidget* wdg)
{
	QVariant rt = wdg->property( "widgetid");
	if (rt.isValid())
	{
		m_objectName = getObjectNameFromWidgetId( rt.toString());
		m_cnt = getCntWidgetId( rt.toString());
	}
}

WidgetId::WidgetId( const QString& str)
{
	m_objectName = getObjectNameFromWidgetId( str);
	m_cnt = getCntWidgetId( str);
}
