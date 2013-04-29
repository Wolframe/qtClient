#include "WidgetVisitor_QRadioButton.hpp"
#include "WidgetVisitor.hpp"
#include "WidgetListener.hpp"
#include "WidgetEnabler.hpp"
#include <QDebug>

WidgetVisitorState_QRadioButton::WidgetVisitorState_QRadioButton( QWidget* widget_)
	:WidgetVisitorObject(widget_)
	,m_radioButton(qobject_cast<QRadioButton*>( widget_))
{
}

bool WidgetVisitorState_QRadioButton::enter( const QString& /*name*/, bool /*writemode*/)
{
	return false;
}

bool WidgetVisitorState_QRadioButton::leave( bool /*writemode*/)
{
	return false;
}

void WidgetVisitorState_QRadioButton::clear()
{
	m_radioButton->setChecked( false);
}

QVariant WidgetVisitorState_QRadioButton::property( const QString& name)
{
	if (name.isEmpty())
	{
		return QVariant( m_radioButton->isChecked());
	}
	return QVariant();
}

bool WidgetVisitorState_QRadioButton::setProperty( const QString& name, const QVariant& data)
{
	if (name.isEmpty())
	{
		m_radioButton->setChecked( data.toBool());
	}
	return false;
}

void WidgetVisitorState_QRadioButton::setState( const QVariant& state)
{
	qDebug() << "set state for radio button" << m_radioButton->objectName();
	if (state.isValid()) m_radioButton->setChecked( state.toBool());
}

QVariant WidgetVisitorState_QRadioButton::getState() const
{
	return QVariant( m_radioButton->isChecked());
}

void WidgetVisitorState_QRadioButton::connectDataSignals( WidgetListener::DataSignalType dt, WidgetListener& listener)
{
	switch (dt)
	{
		case WidgetListener::SigChanged:
			QObject::connect( m_radioButton, SIGNAL( clicked( bool)), &listener, SLOT( changed()), Qt::UniqueConnection);
			QObject::connect( m_radioButton, SIGNAL( released()), &listener, SLOT( changed()), Qt::UniqueConnection);
			QObject::connect( m_radioButton, SIGNAL( toggled( bool)), &listener, SLOT( changed()), Qt::UniqueConnection);
			break;
		case WidgetListener::SigPressed:
			QObject::connect( m_radioButton, SIGNAL( pressed()), &listener, SLOT( pressed()), Qt::UniqueConnection); break;
		case WidgetListener::SigActivated:
		case WidgetListener::SigEntered:
		case WidgetListener::SigClicked:
		case WidgetListener::SigDoubleClicked:
			qCritical() << "try to connect to signal not provided" << m_radioButton->metaObject()->className() << WidgetListener::dataSignalTypeName(dt);
	}
}

void WidgetVisitorState_QRadioButton::connectWidgetEnabler( WidgetEnabler& enabler)
{
	QObject::connect( m_radioButton, SIGNAL( clicked( bool)), &enabler, SLOT( changed()), Qt::UniqueConnection);
	QObject::connect( m_radioButton, SIGNAL( released()), &enabler, SLOT( changed()), Qt::UniqueConnection);
	QObject::connect( m_radioButton, SIGNAL( toggled( bool)), &enabler, SLOT( changed()), Qt::UniqueConnection);
	QObject::connect( m_radioButton, SIGNAL( pressed()), &enabler, SLOT( changed()), Qt::UniqueConnection);
}


