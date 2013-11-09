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

	if (wdg->objectName().size() && !wdg->objectName().startsWith("qt_"))
	{
		WidgetId wid( wdg->objectName(), ++g_cnt);
		QString widprop( wid.toString());
		wdg->setProperty( "widgetid", widprop);
		return widprop;
	}
	else
	{
		return wdg->objectName();
	}
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

WidgetId::WidgetId( const QString& objectName_, int cnt_)
	:m_objectName(objectName_),m_cnt(cnt_)
{}

QString WidgetId::toString() const
{
	QString rt( m_objectName);
	rt.append( ":");
	rt.append( QVariant( m_cnt).toString());
	return rt;
}

