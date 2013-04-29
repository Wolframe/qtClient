#include "WidgetVisitor_QDoubleSpinBox.hpp"
#include "WidgetVisitor.hpp"
#include "WidgetListener.hpp"
#include "WidgetEnabler.hpp"
#include <QDebug>

WidgetVisitorState_QDoubleSpinBox::WidgetVisitorState_QDoubleSpinBox( QWidget* widget_)
	:WidgetVisitorObject(widget_)
	,m_doubleSpinBox(qobject_cast<QDoubleSpinBox*>( widget_))
{
}

bool WidgetVisitorState_QDoubleSpinBox::enter( const QString& /*name*/, bool /*writemode*/)
{
	return false;
}

bool WidgetVisitorState_QDoubleSpinBox::leave( bool /*writemode*/)
{
	return false;
}

void WidgetVisitorState_QDoubleSpinBox::clear()
{
	// set to value between minimum and maximum
	double midval = (m_doubleSpinBox->minimum() + m_doubleSpinBox->maximum()) / 2;
	int stp = (int)(midval / m_doubleSpinBox->singleStep());
	double initval = m_doubleSpinBox->minimum() + stp * m_doubleSpinBox->singleStep();
	m_doubleSpinBox->setValue( initval);
}

QVariant WidgetVisitorState_QDoubleSpinBox::property( const QString& name)
{
	if (name.isEmpty())
	{
		return m_doubleSpinBox->value();
	}
	return QVariant();
}

bool WidgetVisitorState_QDoubleSpinBox::setProperty( const QString& name, const QVariant& data)
{
	if (name.isEmpty())
	{
		m_doubleSpinBox->setValue( data.toDouble());
	}
	return false;
}

void WidgetVisitorState_QDoubleSpinBox::setState( const QVariant& /*state*/)
{
}

QVariant WidgetVisitorState_QDoubleSpinBox::getState() const
{
	return QVariant();
}

void WidgetVisitorState_QDoubleSpinBox::connectDataSignals( WidgetListener::DataSignalType dt, WidgetListener& listener)
{
	switch (dt)
	{
		case WidgetListener::SigChanged:
			QObject::connect( m_doubleSpinBox, SIGNAL( valueChanged( double)), &listener, SLOT( changed()), Qt::UniqueConnection); break;
		case WidgetListener::SigActivated:
		case WidgetListener::SigEntered:
		case WidgetListener::SigPressed:
		case WidgetListener::SigClicked:
		case WidgetListener::SigDoubleClicked:
			qCritical() << "try to connect to signal not provided" << m_doubleSpinBox->metaObject()->className() << WidgetListener::dataSignalTypeName(dt);
	}
}

void WidgetVisitorState_QDoubleSpinBox::connectWidgetEnabler( WidgetEnabler& enabler)
{
	QObject::connect( m_doubleSpinBox, SIGNAL( valueChanged( double)), &enabler, SLOT( changed()), Qt::UniqueConnection);
}


