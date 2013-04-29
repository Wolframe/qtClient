#include "WidgetVisitorObject.hpp"
#include <cstring>
#include <QDebug>

bool WidgetListener::getDataSignalType( const char* name, DataSignalType& dt)
{
	const char* signame;
	for (int ii=0; 0!=(signame=dataSignalTypeName((DataSignalType)ii)); ++ii)
	{
		if (std::strcmp( name, signame) == 0)
		{
			dt = (DataSignalType)ii;
			return true;
		}
	}
	return false;
}

WidgetVisitorObject::WidgetVisitorObject( const WidgetVisitorObject& o)
	:m_widget(o.m_widget)
	,m_synonyms(o.m_synonyms)
	,m_links(o.m_links)
	,m_assignments(o.m_assignments)
	,m_datasignals(o.m_datasignals)
	,m_dataslots(o.m_dataslots)
	,m_dynamicProperties(o.m_dynamicProperties)
	,m_synonym_entercnt(o.m_synonym_entercnt)
	,m_internal_entercnt(o.m_internal_entercnt){}

WidgetVisitorObject::WidgetVisitorObject( QWidget* widget_)
	:m_widget(widget_)
	,m_synonym_entercnt(1)
	,m_internal_entercnt(0)
{
	foreach (const QByteArray& prop, m_widget->dynamicPropertyNames())
	{
		if (prop.indexOf(':') >= 0)
		{
			if (prop.startsWith( "synonym:"))
			{
				QVariant synonym = m_widget->property( prop);
				m_synonyms.insert( prop.mid( 8, prop.size()-8), synonym.toString().trimmed());
			}
			else if (prop.startsWith( "link:"))
			{
				QVariant link = m_widget->property( prop);
				m_links.push_back( LinkDef( prop.mid( 5, prop.size()-5), link.toString().trimmed()));
			}
			else if (prop.startsWith( "assign:"))
			{
				QVariant value = m_widget->property( prop);
				m_assignments.push_back( Assignment( prop.mid( 7, prop.size()-7), value.toString().trimmed()));
			}
			else if (prop.startsWith( "global:"))
			{
				QVariant value = m_widget->property( prop);
				m_globals.push_back( Assignment( prop.mid( 7, prop.size()-7), value.toString().trimmed()));
			}
			else if (prop.startsWith( "datasignal:"))
			{
				const char* signalname = (const char*)prop + 11/*std::strlen( "datasignal:")*/;
				QList<QString> values;
				foreach (const QString& vv, m_widget->property( prop).toString().trimmed().split(','))
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
						qCritical() << "error widget visitor state" << widget_->metaObject()->className() << widget_->objectName() << ": defined unknown data signal name" << prop;
					}
				}
			}
		}
		if (!prop.startsWith( "_w_") && !prop.startsWith( "_q_"))
		{
			m_dynamicProperties.insert( prop, m_widget->property( prop));
		}
	}
	QVariant dataslots = m_widget->property( "dataslot");
	if (dataslots.isValid())
	{
		foreach (const QString& vv, dataslots.toString().trimmed().split(','))
		{
			m_dataslots.push_back( vv.trimmed());
		}
	}
	static qint64 g_cnt = 0;
	QVariant ruid = m_widget->property( "widgetid");
	if (!ruid.isValid())
	{
		QString rt =  m_widget->objectName();
		rt.append( ":");
		rt.append( QVariant( ++g_cnt).toString());
		m_widget->setProperty( "widgetid", QVariant(rt));
	}
}

void WidgetVisitorObject::connectDataSignals( WidgetListener::DataSignalType dt, WidgetListener& /*listener*/)
{
	qCritical() << "try to connect to signal not provided" << m_widget->metaObject()->className() << WidgetListener::dataSignalTypeName(dt);
}

QVariant WidgetVisitorObject::dynamicProperty( const QString& name) const
{
	QHash<QString,QVariant>::const_iterator di = m_dynamicProperties.find( name);
	if (di == m_dynamicProperties.end()) return QVariant();
	return di.value();
}

bool WidgetVisitorObject::setDynamicProperty( const QString& name, const QVariant& value)
{
	m_dynamicProperties.insert( name, value);
	m_widget->setProperty( name.toLatin1(), value);
	return true;
}

QVariant WidgetVisitorObject::getSynonym( const QString& name) const
{
	static const QString empty;
	QHash<QString,QString>::const_iterator syi = m_synonyms.find( name);
	if (syi == m_synonyms.end()) return QVariant();
	return QVariant( syi.value());
}

QString WidgetVisitorObject::getLink( const QString& name) const
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



