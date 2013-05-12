/************************************************************************

 Copyright (C) 2011 - 2013 Project Wolframe.
 All rights reserved.

 This file is part of Project Wolframe.

 Commercial Usage
    Licensees holding valid Project Wolframe Commercial licenses may
    use this file in accordance with the Project Wolframe
    Commercial License Agreement provided with the Software or,
    alternatively, in accordance with the terms contained
    in a written agreement between the licensee and Project Wolframe.

 GNU General Public License Usage
    Alternatively, you can redistribute this file and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Wolframe is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Wolframe.  If not, see <http://www.gnu.org/licenses/>.

 If you have questions regarding the use of this file, please contact
 Project Wolframe.

************************************************************************/

#include "FormWidget.hpp"
#include "FormCall.hpp"
#include "WidgetMessageDispatcher.hpp"
#include "WidgetRequest.hpp"
#include "global.hpp"

#include <QDebug>
#include <QApplication>
#include <QPushButton>
#include <QHBoxLayout>
#include <QFrame>
#include <QList>
#include <QBuffer>

#include <QPluginLoader>
#include <QApplication>
#include <QMainWindow>

FormWidget::FormWidget( FormLoader *_formLoader, DataLoader *_dataLoader, QHash<QString,QVariant>* _globals, QUiLoader *_uiLoader, QWidget *_parent, bool _debug, const QString &_formDir, WolframeClient *_wolframeClient, bool _mdi )
	: QWidget( _parent ), m_form( ),
	  m_uiLoader( _uiLoader ), m_formLoader( _formLoader ),
	  m_dataLoader( _dataLoader ), m_ui( 0 ),
	  m_locale( DEFAULT_LOCALE ), m_layout( 0 ), m_forms( ),
	  m_globals( _globals ), m_widgetTree( _dataLoader, _globals, _debug ),
	  m_debug( _debug ), m_modal( false ), m_newWindow( false ),
	  m_mdi( _mdi ),
	  m_formDir( _formDir ), m_wolframeClient( _wolframeClient )
{
	initialize();
}

void FormWidget::initialize( )
{
	if( !m_layout ) {
		m_layout = new QHBoxLayout( this );
	}

// link the form loader for form loader notifications
	connect( m_formLoader, SIGNAL( formLoaded( QString, QByteArray ) ),
		this, SLOT( formLoaded( QString, QByteArray ) ), Qt::UniqueConnection );
	connect( m_formLoader, SIGNAL( formLocalizationLoaded( QString, QByteArray ) ),
		this, SLOT( formLocalizationLoaded( QString, QByteArray ) ) );
	connect( m_formLoader, SIGNAL( formListLoaded( QStringList ) ),
		this, SLOT( formListLoaded( QStringList ) ) );

// link the data loader to our form widget
	connect( m_dataLoader, SIGNAL( answer( const QString&, const QByteArray& ) ),
		this, SLOT( gotAnswer( const QString&, const QByteArray& ) ), Qt::UniqueConnection );
	connect( m_dataLoader, SIGNAL( error( const QString&, const QByteArray& ) ),
		this, SLOT( gotError( const QString&, const QByteArray& ) ), Qt::UniqueConnection );

// signal dispatcher for form buttons
	m_signalMapper = new QSignalMapper( this );

// the form must be switched after 'action' has been taken in the current form
	connect( m_signalMapper, SIGNAL( mapped( QWidget * ) ),
		this, SLOT( executeAction( QWidget * ) ), Qt::UniqueConnection );
}

void FormWidget::formListLoaded( QStringList forms )
{
// remember list of forms, so we can connect buttons to them
	m_forms = forms;
}

void FormWidget::executeAction( QWidget *actionwidget )
{
	QVariant action = actionwidget->property( "action");

	if (action.isValid())
	{
		WidgetVisitor visitor( actionwidget);
		QPushButton* button = qobject_cast<QPushButton*>( actionwidget);
		if (button->isDown())
		{
			qDebug() << "button pressed twice" << visitor.objectName();
			return;
		}
		WidgetRequest request = getActionRequest( visitor, action, m_debug);

		if (!request.content.isEmpty())
		{
			m_dataLoader->datarequest( request.cmd, request.tag, request.content);
			button->setDown( true);
		}
		if (actionwidget->property( "datasignal:clicked").isValid())
		{
			WidgetListenerR listener( visitor.createListener( m_dataLoader));
			listener->setDebug( m_debug);
			listener->handleDataSignal( WidgetListener::SigClicked);
		}
	}
	else
	{
		switchForm( actionwidget);
	}
}

void FormWidget::executeMenuAction( QWidget *actionwidget, const QString& menuaction)
{
	qDebug() << "execute menu action" << menuaction;

	WidgetVisitor visitor( actionwidget);
	QString suffix;
	QVariant action = actionwidget->property( QByteArray( "action") + ":" + menuaction.toLatin1());

	if (action.isValid())
	{
		WidgetRequest request = getActionRequest( visitor, action, m_debug, menuaction);
		if (!request.content.isEmpty())
		{
			m_dataLoader->datarequest( request.cmd, request.tag, request.content);
		}
	}
	else
	{
		switchForm( actionwidget, menuaction);
	}
}

void FormWidget::switchForm( QWidget *actionwidget, const QString& followform)
{
	m_widgetTree.saveVariables();

	// switch form now, formLoaded will inform parent and others
	WidgetVisitor visitor( actionwidget);
	QVariant formlink;
	if (followform.isEmpty())
	{
		formlink = visitor.property( "form");
	}
	else
	{
		formlink = visitor.property( QString("form") + ":" + followform);
	}
	if (formlink.isValid())
	{
		qDebug() << "Switch form to" << formlink;
		QString nextForm = formlink.toString();

		if (nextForm == "_RESET_")
		{
			//... _RESET_ calls loadForm with the top form of the
			//     form stack and an empty formstate
			if (m_modal)
			{
				qCritical() << "illegal _RESET_ load of modal dialog";
				emit closed( );
			}
			else
			{
				nextForm = m_widgetTree.popCurrentForm();
				if (nextForm.isEmpty())
				{
					qCritical() << "_RESET_ of form but loaded form stack is missing";
					loadForm( m_form);
				}
				else
				{
					loadForm( nextForm);
				}
			}
		}
		else if (nextForm == "_CLOSE_")
		{
			if (m_modal || m_newWindow)
			{
				emit closed();
			}
			else
			{
				nextForm = m_widgetTree.popPreviousForm();
				if (nextForm.isEmpty())
				{
					emit closed();
				}
				else
				{
					loadForm( nextForm);
				}
			}
		}
		else if (m_modal)
		{
			qCritical() << "illegal form call in modal dialog (only _CLOSE_ as next form allowed)";
		}
		else
		{
			// before the load of the next form we store the current
			// state of the form on the state stack so that the
			// _CLOSE_ of the opened form can invoke a restore
			// of the state
			m_widgetTree.saveWidgetStates();
			loadForm( nextForm );
		}
	}
}

void FormWidget::reload( )
{
	loadForm( m_form );
}

FormWidget::~FormWidget( )
{
	if( m_ui ) delete m_ui;
}

void FormWidget::setFormCall( const QString &_form )
{
	loadForm( _form );
}

QString FormWidget::formCall( ) const
{
	return m_form;
}

QIcon FormWidget::getWindowIcon( ) const
{
	return m_ui->windowIcon( );
}

void FormWidget::loadForm( QString name, bool modal, bool newWindow )
{
	if( !m_formLoader ) return;

	m_previousForm = m_form;
	m_form = name;
	m_modal = modal;
	m_newWindow = newWindow;

	qDebug( ) << "Initiating form load for " << m_form << ","
		<< ( m_modal ? "modal" : "non-modal" ) << ","
		<< ( m_newWindow ? "new window" : "same window" );

	m_formLoader->initiateFormLoad( m_form );
}

void FormWidget::setLocale( QLocale locale_ )
{
	m_locale = locale_;
}

void FormWidget::setLanguage( QString language )
{
	setLocale( QLocale( language ) );
}

void FormWidget::changeEvent( QEvent* _event )
{
	if( _event ) {
		if ( _event->type( ) == QEvent::LanguageChange )
			m_ui->update( );
	}

	QWidget::changeEvent( _event );
}

void FormWidget::formLocalizationLoaded( QString name, QByteArray localization )
{
	qDebug( ) << "Form localization loaded for " << name
		<< ", locale " << m_locale.name( )
		<< ", size " << localization.length( );

	qApp->removeTranslator( &m_translator );

	if( m_locale.name( ) != DEFAULT_LOCALE ) {
		if( !m_translator.load( (const uchar *)localization.constData( ), localization.length( ) ) ) {
			qWarning( ) << "Error while loading translations for form " <<
				name << " for locale " << m_locale.name( );
			return;
		}
		qApp->installTranslator( &m_translator );
	}

	QEvent ev( QEvent::LanguageChange );
	qApp->sendEvent( qApp, &ev );

// signal completion of form loading
	qDebug( ) << "Done loading form" << name;
	emit formLoaded( m_form );
}


FormPluginInterface *FormWidget::formPlugin( QString name ) const
{
	QDir pluginDir( m_formDir );
	foreach( QString filename, pluginDir.entryList( QDir::Files ) ) {
		if( !QLibrary::isLibrary( filename ) ) continue;
		QPluginLoader loader( pluginDir.absoluteFilePath( filename ) );
		QObject *object = loader.instance( );
		if( object ) {
			if( qobject_cast<FormPluginInterface *>( object ) ) {
				FormPluginInterface *plugin = qobject_cast<FormPluginInterface *>( object );
				if( plugin->name( ) == name ) {
					return plugin;
				}
			}
		}
	}
	
	return 0;
}

void FormWidget::formLoaded( QString name, QByteArray formXml )
{	
// that's not us
	if( name != m_form ) return;

	qDebug( ) << "Form " << name << " loaded";
	FormCall formCall( name);
	
	QWidget *oldUi = m_ui;
	if( formXml.size( ) == 0 ) {
// byte array 0 indicates no UI description, so we call the plugin
		FormPluginInterface *plugin = formPlugin( formCall.name( ) );
		if( !plugin ) {
			if( !oldUi ) oldUi = new QLabel( "error", this );
			m_ui = oldUi;
			m_form = m_previousForm;
			emit error( tr( "Unable to load form plugin '%1', does the plugin exist?" ).arg( formCall.name( ) ) );
			return;
		}
		qDebug( ) << "PLUGIN: Creating a form plugin with name" << name;
		m_ui = plugin->createForm( formCall, m_dataLoader, m_debug, m_globals, this );
		if( m_ui == 0 ) {
			if( !oldUi ) oldUi = new QLabel( "error", this );
			m_ui = oldUi;
			m_form = m_previousForm;
			emit error( tr( "Unable to initialize form plugin '%1', something went wrong in plugin initialization!" ).arg( formCall.name( ) ) );
			return;
		}
// add new form to layout (which covers the whole widget)
		m_layout->addWidget( m_ui );

		qDebug( ) << "set window title" << plugin->windowTitle( );
		setWindowTitle( plugin->windowTitle( ) );

		if ( oldUi ) {
			m_ui->move( oldUi->pos( ) );
			oldUi->hide( );
			oldUi->deleteLater( );
			oldUi->setParent( 0 );
		}
		m_ui->show( );
		
		return;
	} else {		
// read the form and construct it from the UI file
		QBuffer buf( &formXml );
		m_ui = m_uiLoader->load( &buf, this );
		if( m_ui == 0 ) {
// something went wrong loading or constructing the form
			if( !oldUi ) oldUi = new QLabel( "error", this );
			m_ui = oldUi;
			m_form = m_previousForm;
			emit error( tr( "Unable to load form '%1', does it exist?" ).arg( name ) );
			return;
		}
		buf.close( );
		qDebug( ) << "Constructed UI form XML for form" << name << ","
			<< ( m_modal ? "modal" : "non-modal" ) << ","
			<< ( m_newWindow ? "new window" : "same window" );
	}

// special case of a QMainWindow (we abuse it as menu editor for now),
// show nothing, menu is drawn in main menu when logging in
	if( qobject_cast<QMainWindow *>( m_ui ) ) {
		if( !oldUi ) oldUi = new QLabel( "error", this );
		m_ui = oldUi;
		m_form = m_previousForm;
		emit error( tr( "Calling the menu UI %1 as if it were a normal form. This is a programming mistake!" ).arg( name ) );
		return;
	}
	
// if we have a modal dialog, we must not replace our own form, but emit
// a signal, so the main window can rearange and load the form modal in
// a new window
	if( !m_modal && ( m_ui->isModal( ) || m_ui->isWindow( ) ) ) {
		if( !oldUi ) oldUi = new QLabel( "error", this );
		m_ui = oldUi;
		m_form = m_previousForm;
		emit formModal( name );
		return;
	}

// if the window is not a singleton, the main window must open our form
// in a new MDI window eventually
	if( m_mdi && !m_modal && !m_newWindow && !m_ui->property( "singletonWindow" ).toBool( )
		&& m_form != m_previousForm ) {
		if( !oldUi ) oldUi = new QLabel( "error", this );
		m_ui = oldUi;
		m_form = m_previousForm;
		emit formNewWindow( name );
		return;
	}

// initialize the form data
	m_widgetTree.initialize( m_ui, oldUi, m_form);

// add new form to layout (which covers the whole widget)
	m_layout->addWidget( m_ui );

	qDebug( ) << "set window title" << m_ui->windowTitle( );
	setWindowTitle( m_ui->windowTitle( ) );

	if ( oldUi ) {
		m_ui->move( oldUi->pos( ) );
		oldUi->hide( );
		oldUi->deleteLater( );
		oldUi->setParent( 0 );
	}
	m_ui->show( );

// set localization now
	qDebug( ) << "Starting to load localization for form" << name;
	m_formLoader->initiateFormLocalizationLoad( m_form, m_locale );

// connect push buttons with form names to loadForms
	QList<QWidget *> widgets = m_ui->findChildren<QWidget *>( );
	foreach( QWidget *widget, widgets )
	{
		QPushButton *pushButton = qobject_cast<QPushButton*>( widget);
		if (pushButton)
		{
			// connect button
			connect( pushButton, SIGNAL( clicked( ) ),
				m_signalMapper, SLOT( map( ) ), Qt::UniqueConnection);

			m_signalMapper->setMapping( pushButton, widget );
		}
	}

// load localization of the form now
	qDebug( ) << "Initiating form locatization load for " << m_form << " and locale "
		<< m_locale.name( );
	m_formLoader->initiateFormLocalizationLoad( m_form, m_locale );
}

void FormWidget::gotAnswer( const QString& tag_, const QByteArray& data_)
{
// hand-written plugin, custom request, pass it back directly, don't go over
// generic widget answer part (TODO: there should be a registry map here perhaps)
	FormPluginInterface *plugin = formPlugin( FormCall::name( m_form ) );
	if( plugin ) {
		plugin->gotAnswer( tag_, data_ );
		return;
	}

	QString followform;
	QWidget* rcp = m_widgetTree.deliverAnswer( tag_, data_, followform);
	if (rcp)
	{
		qDebug( ) << "Answer for form" << m_form << "and tag" << tag_;

		if (!followform.isEmpty() || rcp->property( "form").isValid())
		{
			WidgetVisitor actionvisitor( rcp);
			FormWidget* THIS_ = actionvisitor.formwidget();
			THIS_->switchForm( rcp, followform);
		}
	}
}

void FormWidget::gotError( const QString& tag_, const QByteArray& data_)
{
// hand-written plugin, custom request, pass it back directly, don't go over
// generic widget answer part (TODO: there should be a registry map here perhaps)
	FormPluginInterface *plugin = formPlugin( FormCall::name( m_form ) );
	if( plugin ) {
		plugin->gotError( tag_, data_ );
		return;
	}

	if (m_widgetTree.deliverError( tag_, data_))
	{
		qDebug( ) << "Error for form" << m_form << "and tag" << tag_;
		emit error( QString( data_));
	}
}

void FormWidget::closeEvent( QCloseEvent *e )
{
	emit closed( );
	e->accept( );
}

QVariant FormWidget::getWidgetStates() const
{
	return m_widgetTree.getWidgetStates();
}

void FormWidget::setWidgetStates( const QVariant& state)
{
	m_widgetTree.setWidgetStates( state);
}


