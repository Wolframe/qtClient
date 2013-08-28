#include "WidgetVisitor_QStackedWidget.hpp"
#include "WidgetVisitor.hpp"
#include "WidgetListener.hpp"
#include "WidgetEnabler.hpp"
#include <QDebug>

WidgetVisitorState_QStackedWidget::WidgetVisitorState_QStackedWidget( QWidget* widget_)
	:WidgetVisitorObject(widget_)
	,m_stackedWidget(qobject_cast<QStackedWidget*>( widget_))
{
}

bool WidgetVisitorState_QStackedWidget::enter( const QString& /*name*/, bool /*writemode*/)
{
	return false;
}

bool WidgetVisitorState_QStackedWidget::leave( bool /*writemode*/)
{
	return false;
}

void WidgetVisitorState_QStackedWidget::clear()
{
	// do nothing here
}

QVariant WidgetVisitorState_QStackedWidget::property( const QString& name)
{
	static const QString page_str( "page" );

	if( name == page_str ) {
		return m_stackedWidget->currentWidget( )->objectName( );
	}

	return QVariant( );
}

bool WidgetVisitorState_QStackedWidget::setProperty( const QString& name, const QVariant& data)
{
	static const QString page_str( "page" );

	if( name == page_str ) {
		QObjectList children = m_stackedWidget->children( );
		foreach( QObject *child, children ) {
			QWidget *widget = qobject_cast<QWidget *>( child );
			if( widget ) {
				if( data.toString( ) == widget->objectName( ) ) {
					m_stackedWidget->setCurrentWidget( widget );
				}
			}
		}
		return true;
	}

	return false;
}

void WidgetVisitorState_QStackedWidget::setState( const QVariant& /*state*/)
{
	qDebug() << "set state for stacked widget" << m_stackedWidget->objectName();
}

QVariant WidgetVisitorState_QStackedWidget::getState() const
{
	return QVariant();
}

void WidgetVisitorState_QStackedWidget::connectDataSignals( WidgetListener::DataSignalType dt, WidgetListener& listener)
{
	switch (dt)
	{
		case WidgetListener::SigChanged:
			QObject::connect( m_stackedWidget, SIGNAL( currentChanged( int)), &listener, SLOT( changed()), Qt::UniqueConnection); break;
		case WidgetListener::SigActivated:
		case WidgetListener::SigEntered:
		case WidgetListener::SigPressed:
		case WidgetListener::SigClicked:
		case WidgetListener::SigDoubleClicked:
			qCritical() << "try to connect to signal not provided" << m_stackedWidget->metaObject()->className() << WidgetListener::dataSignalTypeName(dt);
		case WidgetListener::SigDestroyed: break;
	}
}

void WidgetVisitorState_QStackedWidget::connectWidgetEnabler( WidgetEnabler& enabler)
{
	QObject::connect( m_stackedWidget, SIGNAL( currentChanged( int)), &enabler, SLOT( changed()), Qt::UniqueConnection);
}


