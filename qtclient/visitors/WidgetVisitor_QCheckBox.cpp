#include "WidgetVisitor_QCheckBox.hpp"
#include "WidgetVisitor.hpp"
#include "WidgetListener.hpp"
#include "WidgetEnabler.hpp"
#include <QDebug>

WidgetVisitorState_QCheckBox::WidgetVisitorState_QCheckBox( QWidget* widget_)
	:WidgetVisitorObject(widget_)
	,m_checkBox(qobject_cast<QCheckBox*>( widget_))
{
}

bool WidgetVisitorState_QCheckBox::enter( const QString& /*name*/, bool /*writemode*/)
{
	return false;
}

bool WidgetVisitorState_QCheckBox::leave( bool /*writemode*/)
{
	return false;
}

void WidgetVisitorState_QCheckBox::clear()
{
	m_checkBox->setChecked( false);
}

QVariant WidgetVisitorState_QCheckBox::property( const QString& name)
{
	if (name.isEmpty())
	{
		return QVariant( m_checkBox->isChecked());
	}
	return QVariant();
}

bool WidgetVisitorState_QCheckBox::setProperty( const QString& name, const QVariant& data)
{
	if (name.isEmpty())
	{
		m_checkBox->setChecked( data.toBool());
	}
	return false;
}

void WidgetVisitorState_QCheckBox::setState( const QVariant& state)
{
	if (state.isValid()) m_checkBox->setCheckState( (Qt::CheckState)state.toInt());
}

QVariant WidgetVisitorState_QCheckBox::getState() const
{
	return QVariant( (int)m_checkBox->checkState());
}

void WidgetVisitorState_QCheckBox::connectDataSignals( WidgetListener::DataSignalType dt, WidgetListener& listener)
{
	switch (dt)
	{
		case WidgetListener::SigChanged:
			QObject::connect( m_checkBox, SIGNAL( clicked( bool)), &listener, SLOT( changed()), Qt::UniqueConnection);
			QObject::connect( m_checkBox, SIGNAL( released()), &listener, SLOT( changed()), Qt::UniqueConnection);
			QObject::connect( m_checkBox, SIGNAL( toggled( bool)), &listener, SLOT( changed()), Qt::UniqueConnection);
			break;
		case WidgetListener::SigPressed:
			QObject::connect( m_checkBox, SIGNAL( pressed()), &listener, SLOT( pressed()), Qt::UniqueConnection); break;
		case WidgetListener::SigActivated:
		case WidgetListener::SigEntered:
		case WidgetListener::SigClicked:
		case WidgetListener::SigDoubleClicked:
			qCritical() << "try to connect to signal not provided" << m_checkBox->metaObject()->className() << WidgetListener::dataSignalTypeName(dt);
	}
}

void WidgetVisitorState_QCheckBox::connectWidgetEnabler( WidgetEnabler& enabler)
{
	QObject::connect( m_checkBox, SIGNAL( clicked( bool)), &enabler, SLOT( changed()), Qt::UniqueConnection);
	QObject::connect( m_checkBox, SIGNAL( released()), &enabler, SLOT( changed()), Qt::UniqueConnection);
	QObject::connect( m_checkBox, SIGNAL( toggled( bool)), &enabler, SLOT( changed()), Qt::UniqueConnection);
	QObject::connect( m_checkBox, SIGNAL( pressed()), &enabler, SLOT( changed()), Qt::UniqueConnection);
}


