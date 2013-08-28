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
	QVariant origtext = m_radioButton->property( "_w_origtext");
	if (!origtext.isValid())
	{
		m_radioButton->setProperty( "_w_origtext", origtext = m_radioButton->text());
	}
	m_radioButton->setChecked( false);
	m_radioButton->setText( origtext.toString());
}

QVariant WidgetVisitorState_QRadioButton::property( const QString& name)
{
	if (name.isEmpty())
	{
		return QVariant( m_radioButton->isChecked());
	}
	else if (name == "text")
	{
		return QVariant( m_radioButton->text());
	}
	return QVariant();
}

bool WidgetVisitorState_QRadioButton::setProperty( const QString& name, const QVariant& data)
{
	if (name.isEmpty())
	{
		m_radioButton->setChecked( data.toBool());
	}
	else if (name == "text")
	{
		m_radioButton->setText( data.toString());
		return true;
	}
	else if (name.size() == 1 && name.at(0) >= '1' && name.at(0) <= '9')
	{
		QString subst("%");
		subst.push_back( name.at(0));

		if (!m_radioButton->property( "_w_origtext").isValid())
		{
			m_radioButton->setProperty( "_w_origtext", m_radioButton->text());
		}
		m_radioButton->setText( m_radioButton->text().replace( subst, data.toString()));
		m_radioButton->adjustSize();
		return true;
	}
	return false;
}

void WidgetVisitorState_QRadioButton::setState( const QVariant& state)
{
	qDebug() << "set state for radio button" << m_radioButton->objectName();
	if (state.isValid() && state.type() == QVariant::List)
	{
		QList<QVariant> lst = state.toList();
		if (lst.size() > 0) m_radioButton->setChecked( lst.at(0).toBool());
		if (lst.size() > 1) m_radioButton->setText( lst.at(1).toString());
	}
}

QVariant WidgetVisitorState_QRadioButton::getState() const
{
	QList<QVariant> lst;
	lst.push_back( m_radioButton->isChecked());
	return QVariant( lst);
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
		case WidgetListener::SigDestroyed: break;
	}
}

void WidgetVisitorState_QRadioButton::connectWidgetEnabler( WidgetEnabler& enabler)
{
	QObject::connect( m_radioButton, SIGNAL( clicked( bool)), &enabler, SLOT( changed()), Qt::UniqueConnection);
	QObject::connect( m_radioButton, SIGNAL( released()), &enabler, SLOT( changed()), Qt::UniqueConnection);
	QObject::connect( m_radioButton, SIGNAL( toggled( bool)), &enabler, SLOT( changed()), Qt::UniqueConnection);
	QObject::connect( m_radioButton, SIGNAL( pressed()), &enabler, SLOT( changed()), Qt::UniqueConnection);
}


