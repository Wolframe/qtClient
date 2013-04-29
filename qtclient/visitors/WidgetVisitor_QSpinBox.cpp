#include "WidgetVisitor_QSpinBox.hpp"
#include "WidgetVisitor.hpp"
#include "WidgetListener.hpp"
#include "WidgetEnabler.hpp"
#include <QDebug>

WidgetVisitorState_QSpinBox::WidgetVisitorState_QSpinBox( QWidget* widget_)
	:WidgetVisitorObject(widget_)
	,m_spinBox(qobject_cast<QSpinBox*>( widget_))
{
}

bool WidgetVisitorState_QSpinBox::enter( const QString& /*name*/, bool /*writemode*/)
{
	return false;
}

bool WidgetVisitorState_QSpinBox::leave( bool /*writemode*/)
{
	return false;
}

void WidgetVisitorState_QSpinBox::clear()
{
	// set to value between minimum and maximum
	int midval = (m_spinBox->minimum() + m_spinBox->maximum()) / 2;
	int stp = midval / m_spinBox->singleStep();
	double initval = m_spinBox->minimum() + stp * m_spinBox->singleStep();
	m_spinBox->setValue( initval);
}

QVariant WidgetVisitorState_QSpinBox::property( const QString& name)
{
	if (name.isEmpty())
	{
		return m_spinBox->value();
	}
	return QVariant();
}

bool WidgetVisitorState_QSpinBox::setProperty( const QString& name, const QVariant& data)
{
	if (name.isEmpty())
	{
		m_spinBox->setValue( data.toInt());
	}
	return false;
}

void WidgetVisitorState_QSpinBox::setState( const QVariant& state)
{
	qDebug() << "set state for spin box" << m_spinBox->objectName();
	if (state.isValid()) m_spinBox->setValue( state.toDouble());
}

QVariant WidgetVisitorState_QSpinBox::getState() const
{
	return (m_spinBox->value() != 0 && m_spinBox->value() != m_spinBox->minimum()) ?QVariant( m_spinBox->value()):QVariant();
}

void WidgetVisitorState_QSpinBox::connectDataSignals( WidgetListener::DataSignalType dt, WidgetListener& listener)
{
	switch (dt)
	{
		case WidgetListener::SigChanged:
			QObject::connect( m_spinBox, SIGNAL( valueChanged( int)), &listener, SLOT( changed()), Qt::UniqueConnection); break;
		case WidgetListener::SigActivated:
		case WidgetListener::SigEntered:
		case WidgetListener::SigPressed:
		case WidgetListener::SigClicked:
		case WidgetListener::SigDoubleClicked:
			qCritical() << "try to connect to signal not provided" << m_spinBox->metaObject()->className() << WidgetListener::dataSignalTypeName(dt);
	}
}

void WidgetVisitorState_QSpinBox::connectWidgetEnabler( WidgetEnabler& enabler)
{
	QObject::connect( m_spinBox, SIGNAL( valueChanged( int)), &enabler, SLOT( changed()), Qt::UniqueConnection);
}



