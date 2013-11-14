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

bool WidgetId::isValid( const QString& id)
{
	return id.indexOf(':') >= 0;
}

QString WidgetId::objectName( const QString& id)
{
	int endpos = id.indexOf(':');
	if (endpos < 0) endpos = id.size();
	return id.mid( 0, endpos);
}

int WidgetId::cnt( const QString& id)
{
	int startpos = id.indexOf(':');
	if (startpos < 0) return -1;
	return id.mid( startpos+1).toInt();
}

WidgetId::WidgetId( const QWidget* wdg)
{
	QVariant rt = wdg->property( "widgetid");
	if (rt.isValid())
	{
		m_objectName = WidgetId::objectName( rt.toString());
		m_cnt = WidgetId::cnt( rt.toString());
	}
}

WidgetId::WidgetId( const QString& str)
{
	m_objectName = WidgetId::objectName( str);
	m_cnt = WidgetId::cnt( str);
}

WidgetId::WidgetId( const QString& objectName_, int cnt_)
	:m_objectName(objectName_),m_cnt(cnt_)
{}

WidgetId::WidgetId( const WidgetId& o)
	:m_objectName(o.m_objectName),m_cnt(o.m_cnt)
{}

QString WidgetId::toString() const
{
	QString rt( m_objectName);
	rt.append( ":");
	rt.append( QString::number( m_cnt));
	return rt;
}


