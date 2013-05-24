#include "WidgetVisitor_QSlider.hpp"
#include "WidgetVisitor.hpp"
#include "WidgetListener.hpp"
#include "WidgetEnabler.hpp"
#include <QDebug>

WidgetVisitorState_QSlider::WidgetVisitorState_QSlider( QWidget* widget_)
	:WidgetVisitorObject(widget_)
	,m_slider(qobject_cast<QSlider*>( widget_))
{
}

bool WidgetVisitorState_QSlider::enter( const QString& /*name*/, bool /*writemode*/)
{
	return false;
}

bool WidgetVisitorState_QSlider::leave( bool /*writemode*/)
{
	return false;
}

void WidgetVisitorState_QSlider::clear()
{
	// set to value between minimum and maximum
	int midval = (m_slider->minimum() + m_slider->maximum()) / 2;
	int stp = midval / m_slider->singleStep();
	double initval = m_slider->minimum() + stp * m_slider->singleStep();
	m_slider->setValue( initval);
}

QVariant WidgetVisitorState_QSlider::property( const QString& name)
{
	if (name.isEmpty())
	{
		return m_slider->value();
	}
	return QVariant();
}

bool WidgetVisitorState_QSlider::setProperty( const QString& name, const QVariant& data)
{
	if (name.isEmpty())
	{
		m_slider->setValue( data.toInt());
	}
	return false;
}

void WidgetVisitorState_QSlider::setState( const QVariant& /*state*/)
{
	qDebug() << "set state for slider" << m_slider->objectName();
}

QVariant WidgetVisitorState_QSlider::getState() const
{
	return QVariant();
}

void WidgetVisitorState_QSlider::connectDataSignals( WidgetListener::DataSignalType dt, WidgetListener& listener)
{
	switch (dt)
	{
		case WidgetListener::SigChanged:
			QObject::connect( m_slider, SIGNAL( valueChanged( int)), &listener, SLOT( changed()), Qt::UniqueConnection); break;
		case WidgetListener::SigActivated:
		case WidgetListener::SigEntered:
		case WidgetListener::SigPressed:
		case WidgetListener::SigClicked:
		case WidgetListener::SigDoubleClicked:
			qCritical() << "try to connect to signal not provided" << m_slider->metaObject()->className() << WidgetListener::dataSignalTypeName(dt);
		case WidgetListener::SigDestroyed: break;
	}
}

void WidgetVisitorState_QSlider::connectWidgetEnabler( WidgetEnabler& enabler)
{
	QObject::connect( m_slider, SIGNAL( valueChanged( int)), &enabler, SLOT( changed()), Qt::UniqueConnection);
}


