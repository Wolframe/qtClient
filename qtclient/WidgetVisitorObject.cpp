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

WidgetVisitorObject::WidgetVisitorObject( QWidget* widget_)
	:m_widget(widget_)
{}

void WidgetVisitorObject::connectDataSignals( WidgetListener::DataSignalType dt, WidgetListener& /*listener*/)
{
	qCritical() << "try to connect to signal not provided" << m_widget->metaObject()->className() << WidgetListener::dataSignalTypeName(dt);
}




