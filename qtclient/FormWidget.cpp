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

static bool isInternalWidget( const QWidget* widget)
{
	return widget->objectName().isEmpty()
		|| widget->objectName().startsWith( "qt_")
		|| widget->objectName().startsWith( "_q");
}

FormWidget::FormWidget( FormLoader *_formLoader, DataLoader *_dataLoader, QHash<QString,QVariant>* _globals, QUiLoader *_uiLoader, QWidget *_parent, bool _debug, const QString &_formDir, WolframeClient *_wolframeClient )
	: QWidget( _parent ), m_form( ),
	  m_uiLoader( _uiLoader ), m_formLoader( _formLoader ),
	  m_dataLoader( _dataLoader ), m_globals(_globals ), m_ui( 0 ),
	  m_locale( DEFAULT_LOCALE ), m_layout( 0 ), m_forms( ),
	  m_debug( _debug ), m_modal( false ), m_newWindow( false ),
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
		this, SLOT( formLoaded( QString, QByteArray ) ) );
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

		WidgetMessageDispatcher dispatcher( visitor);
		WidgetRequest request = dispatcher.getFormActionRequest( m_debug);

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
		WidgetRequest request = getMenuActionRequest( visitor, menuaction, m_debug);
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
	WidgetVisitor formvisitor( m_ui);
	formvisitor.do_writeAssignments();
	formvisitor.do_writeGlobals( *m_globals);

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
		m_formstate = QVariant();

		if (nextForm == "_RESET_")
		{
			//... _RELOAD_ calls loadForm with the top form of the
			//     form stack and an empty formstate
			if (m_modal)
			{
				qCritical() << "illegal _RELOAD_ load of modal dialog";
				emit closed( );
			}
			else
			{
				QList<QVariant> formstack = m_ui->property("_w_formstack").toList();
				QList<QVariant> statestack = m_ui->property("_w_statestack").toList();

				if (formstack.isEmpty())
				{
					qCritical() << "_RELOAD_ of form but loaded form stack is missing";
					loadForm( m_form);
				}
				else
				{
					qDebug() << "form stack before pop (_RELOAD_):" << formstack;

					nextForm = formstack.back().toString();
					formstack.pop_back();
					if (!statestack.isEmpty())
					{
						statestack.pop_back();
					}
					m_ui->setProperty( "_w_formstack", QVariant( formstack));
					m_ui->setProperty( "_w_statestack", QVariant( statestack));

					qDebug() << "load form from stack:" << nextForm;
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
				//... _CLOSE_ calls loadForm with the predecessor of
				//    the top form of the form stack and the formstate
				//    stored there (parallel on the statestack).
				QList<QVariant> formstack = m_ui->property("_w_formstack").toList();
				QList<QVariant> statestack = m_ui->property("_w_statestack").toList();

				if (!formstack.isEmpty())
				{
					qDebug() << "form stack before pop (_CLOSE_):" << formstack;

					// discard the top element of the form/state stack
					formstack.pop_back();
					if (!statestack.isEmpty())
					{
						statestack.pop_back();
					}
					if (formstack.isEmpty())
					{
						emit closed();
					}
					else
					{
						// fetch the predecessor of the form/state stack
						// as next form+state
						nextForm = formstack.back().toString();
						if (!formstack.isEmpty())
						{
							formstack.pop_back();
						}
						if (!statestack.isEmpty())
						{
							m_formstate = statestack.back();
							statestack.pop_back();
						}
						m_ui->setProperty( "_w_formstack", QVariant( formstack));
						m_ui->setProperty( "_w_statestack", QVariant( statestack));

						qDebug() << "load form from stack:" << nextForm;
						loadForm( nextForm);
					}
				}
				else
				{
					emit closed();
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
			QList<QVariant> statestack = m_ui->property("_w_statestack").toList();
			if (!statestack.isEmpty())
			{
				// replace state of the state stack:
				statestack.pop_back();
				statestack.push_back( getWidgetStates());
				m_ui->setProperty( "_w_statestack", QVariant( statestack));
			}
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

static bool nodeProperty_hasListener( const QWidget* widget, const QVariant&)
{
	if (WidgetListenerImpl::hasDataSignals( widget)) return true;
	if (widget->property( "contextmenu").isValid()) return true;
	return false;
}

static bool nodeProperty_hasDebugListener( const QWidget* widget, const QVariant&)
{
	if (WidgetListenerImpl::hasDataSignals( widget)) return true;
	if (widget->property( "contextmenu").isValid()) return true;
	if (widget->property( "action").isValid()) return true;
	if (widget->property( "form").isValid()) return true;
	foreach (const QByteArray& prop, widget->dynamicPropertyNames())
	{
		if (prop.startsWith( "action:") || prop.startsWith( "form:")) return true;
	}
	return false;
}

void FormWidget::setPushButtonEnablers( QPushButton* pushButton)
{
	QList<QString> enable_props;
	WidgetVisitor button_visitor( pushButton);

	foreach (const QString& prop, getActionRequestProperties( button_visitor))
	{
		if (!enable_props.contains( prop)) enable_props.push_back( prop);
	}
	foreach (const QString& prop, getFormCallProperties( pushButton->property( "form").toString()))
	{
		if (!enable_props.contains( prop)) enable_props.push_back( prop);
	}

	bool enabled = true;
	QHash<QString,WidgetEnablerR> button_enablermap;

	foreach (const QString& prop, enable_props)
	{
		QWidget* ownerwidget = button_visitor.getPropertyOwnerWidget( prop);
		if (ownerwidget)
		{
			WidgetEnablerR enabler;
			QString objName = ownerwidget->objectName();

			QHash<QString,WidgetEnablerR>::const_iterator eni = button_enablermap.find( objName);
			if (eni != button_enablermap.end())
			{
				enabler = eni.value();
			}
			else
			{
				enabler = WidgetEnablerR( new WidgetEnablerImpl( pushButton, enable_props));
				QHash<QString,QList<WidgetEnablerR> >::const_iterator fi = m_enablers.find( objName), fe = m_enablers.end();
				if (fi == fe)
				{
					m_enablers.insert( objName, QList<WidgetEnablerR>());
				}
				button_enablermap[ objName] = enabler;
				m_enablers[ objName].push_back( enabler);
			}
			WidgetVisitor ownervisitor( ownerwidget);
			ownervisitor.connectWidgetEnabler( *enabler);

			if (!button_visitor.property( prop).isValid())
			{
				enabled = false;
			}
		}
		else
		{
			qCritical() << "could not evaluate widget delivering property" << prop;
			enabled = false;
		}
	}
	pushButton->setEnabled( enabled);
}

void FormWidget::signalPushButtonEnablers()
{
	QHash<QString,QList<WidgetEnablerR> >::iterator li = m_enablers.begin(), le = m_enablers.end();
	for (; li != le; ++li)
	{
		QList<WidgetEnablerR>::iterator ei = li->begin(), ee = li->end();
		for (; ei != ee; ++ei) ei->data()->changed();
	}
}

QVariant FormWidget::getWidgetStates() const
{
	if (!m_ui) return QVariant();
	QList<QVariant> rt;
	foreach( QWidget *widget, m_ui->findChildren<QWidget*>())
	{
		if (!isInternalWidget( widget))
		{
			WidgetVisitor visitor( widget);
			QVariant state = visitor.getState();
			if (state.isValid())
			{
				rt.push_back( visitor.objectName());
				rt.push_back( state);
			}
		}
	}
	if (rt.isEmpty()) return QVariant();
	return QVariant(rt);
}

void FormWidget::setWidgetStates( const QVariant& state)
{
	QList<QVariant> statelist = state.toList();
	QList<QVariant>::const_iterator itr = statelist.begin();

	foreach( QWidget *widget, m_ui->findChildren<QWidget*>())
	{
		if (!isInternalWidget( widget))
		{
			if (itr != statelist.end() && widget->objectName() == itr->toString())
			{
				WidgetVisitor visitor( widget);
				++itr;
				if (itr != statelist.end())
				{
					if (visitor.property( "action").isValid())
					{
						widget->setProperty( "_w_state", *itr);
					}
					else
					{
						visitor.setState( *itr);
						QVariant initialFocus = widget->property( "initialFocus");
						if (initialFocus.toBool()) widget->setFocus();
					}
					++itr;
				}
			}
		}
	}
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
		FormPluginInterface *plugin = formPlugin( FormCall::name( name ) );
		if( !plugin ) {
			if( !oldUi ) oldUi = new QLabel( "error", this );
			m_ui = oldUi;
			m_form = m_previousForm;
			emit error( tr( "Unable to load form plugin '%1', does the plugin exist?" ).arg( FormCall::name( name ) ) );
			return;
		}
		qDebug( ) << "PLUGIN: Creating a form plugin with name" << name;
		m_ui = plugin->createForm( FormCall( name ), m_dataLoader, m_debug, m_globals, this );
		if( m_ui == 0 ) {
			if( !oldUi ) oldUi = new QLabel( "error", this );
			m_ui = oldUi;
			m_form = m_previousForm;
			emit error( tr( "Unable to initialize form plugin '%1', something went wrong in plugin initialization!" ).arg( FormCall::name( name ) ) );
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
	if( !m_modal && !m_newWindow && !m_ui->property( "singletonWindow" ).toBool( ) ) {
		if( !oldUi ) oldUi = new QLabel( "error", this );
		m_ui = oldUi;
		m_form = m_previousForm;
		emit formNewWindow( name );
		return;
	}

// initialize the form variables given by globals
	WidgetVisitor::init_widgetids( m_ui);
	WidgetVisitor visitor( m_ui);
	visitor.do_readGlobals( *m_globals);

// initialize the form variables given by form parameters
	foreach (const FormCall::Parameter& param, formCall.parameter())
	{
		if (!visitor.setProperty( QString( param.first), param.second))
		{
			qCritical() << "Failed to set UI parameter" << param.first << "=" << param.second;
		}
		else
		{
			qDebug( ) << "Set UI parameter" << param.first << "=" << param.second;
		}
	}
// initialize the form variables given by assignments
	visitor.do_readAssignments();

	// restore widget states if form was opened with a '_CLOSE_'
	if (m_formstate.isValid())
	{
		setWidgetStates( m_formstate);
	}

// set widget state according to 'state:..' properties and their conditionals
	foreach (QWidget* chld, m_ui->findChildren<QWidget *>())
	{
		foreach (const QByteArray& prop, chld->dynamicPropertyNames())
		{
			if (prop.startsWith( "state:"))
			{
				WidgetVisitor vv( chld);

				QByteArray nam = prop.mid( 6, prop.size()-6);
				QVariant condexpr = chld->property( prop);
				bool cond = vv.evalCondition( condexpr);
				if (nam == "visible")
				{
					qDebug() << "set" << chld->objectName() << "visible" << cond << "because of condition" << condexpr;
					chld->setVisible( cond);
				}
				else if (nam == "invisible")
				{
					qDebug() << "set" << chld->objectName() << "invisible" << cond << "because of condition" << condexpr;
					chld->setVisible( !cond);
				}
				else if (nam == "hidden")
				{
					qDebug() << "set" << chld->objectName() << "hidden" << cond << "because of condition" << condexpr;
					chld->setHidden( cond);
				}
				else if (nam == "enabled")
				{
					qDebug() << "set" << chld->objectName() << "enabled" << cond << "because of condition" << condexpr;
					chld->setEnabled( cond);
				}
				else if (nam == "disabled")
				{
					qDebug() << "set" << chld->objectName() << "disabled" << cond << "because of condition" << condexpr;
					chld->setDisabled( cond);
				}
				else
				{
					qCritical() << "unknown state property name" << prop;
				}
			}
		}
	}


// connect listener to signals converted to data signals
	m_listeners.clear();
	if (m_debug)
	{
		foreach (QWidget* datasig_widget, visitor.findSubNodes( nodeProperty_hasDebugListener))
		{
			WidgetVisitor datasig_widget_visitor( datasig_widget);
			WidgetListenerR listener( datasig_widget_visitor.createListener( m_dataLoader));
			listener->setDebug( m_debug);
			m_listeners[ datasig_widget_visitor.widgetid()].push_back( listener);
		}
	}
	else
	{
		foreach (QWidget* datasig_widget, visitor.findSubNodes( nodeProperty_hasListener))
		{
			WidgetVisitor datasig_widget_visitor( datasig_widget);
			WidgetListenerR listener( datasig_widget_visitor.createListener( m_dataLoader));
			listener->setDebug( m_debug);
			m_listeners[ datasig_widget_visitor.widgetid()].push_back( listener);
		}
	}

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
	m_enablers.clear();

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
			setPushButtonEnablers( pushButton);
		}
	}

// push on form stack for back link
	QList<QVariant> formstack;
	QList<QVariant> statestack;
	if( oldUi )
	{
		formstack = oldUi->property( "_w_formstack").toList();
		statestack = oldUi->property( "_w_statestack").toList();
	}
	formstack.push_back( m_form);
	statestack.push_back( m_formstate );

	qDebug() << "form stack for " << m_form << ":" << formstack;
	m_ui->setProperty( "_w_formstack", QVariant( formstack));
	m_ui->setProperty( "_w_statestack", QVariant( statestack));

// loads the domains
	WidgetMessageDispatcher dispatcher( m_ui);
	foreach (const WidgetRequest& request, dispatcher.getDomainLoadRequests( m_debug))
	{
		if (!request.content.isEmpty())
		{
			m_dataLoader->datarequest( request.cmd, request.tag, request.content);
		}
	}

// load localization of the form now
	qDebug( ) << "Initiating form locatization load for " << m_form << " and locale "
		<< m_locale.name( );
	m_formLoader->initiateFormLocalizationLoad( m_form, m_locale );
}

void FormWidget::gotAnswer( const QString& tag_, const QByteArray& data_)
{
	qDebug( ) << "Answer for form" << m_form << "and tag" << tag_;

// hand-written plugin, custom request, pass it back directly, don't go over
// generic widget answer part (TODO: there should be a registry map here perhaps)
	FormPluginInterface *plugin = formPlugin( FormCall::name( m_form ) );
	if( plugin ) {
		plugin->gotAnswer( tag_, data_ );
		return;
	}
	
	WidgetVisitor visitor( m_ui);
	WidgetMessageDispatcher dispatcher( visitor);
	WidgetRequest rq( tag_);

	if (rq.type() == WidgetRequest::Action)
	{
		QList<QWidget*> rcpl = dispatcher.findRecipients( rq.recipientid());
		if (!rcpl.isEmpty())
		{
			qDebug() << "got action request answer tag=" << tag_ << "data=" << data_;
			foreach (QWidget* actionwidget, rcpl)
			{
				QPushButton* button = qobject_cast<QPushButton*>( actionwidget);
				if (button) button->setDown( false);
				WidgetVisitor actionvisitor( actionwidget);
				FormWidget* THIS_ = actionvisitor.formwidget();
				THIS_->switchForm( actionwidget, rq.followform());
			}
		}
	}
	else
	{
		QList<QWidget*> rcpl = dispatcher.findRecipients( rq.recipientid());
		if (!rcpl.isEmpty())
		{
			foreach (QWidget* rcp, rcpl)
			{
				//block signals before assigning the answer
				typedef QPair<QString,bool> Trace;
				typedef QList<Trace> TraceList;
				TraceList blksig;
				blksig.push_back( Trace( rcp->objectName(), rcp->blockSignals(true)));

				foreach (QWidget* cld, rcp->findChildren<QWidget*>())
				{
					if (!isInternalWidget( cld))
					{
						qDebug() << "block signals of" << cld->metaObject()->className() << cld->objectName();
						blksig.push_back( Trace( cld->objectName(), cld->blockSignals(true)));
					}
				}
				WidgetVisitor rcpvisitor( rcp, (WidgetVisitor::VisitorFlags)(WidgetVisitor::UseSynonyms|WidgetVisitor::BlockSignals));
				if (!setWidgetAnswer( rcpvisitor, data_))
				{
					qCritical() << "Failed assign request answer tag:" << tag_ << "data:" << data_;
				}
				rcpvisitor.setState( rcp->property( "_w_state"));
				QVariant initialFocus = rcp->property( "initialFocus");
				if (initialFocus.toBool()) rcp->setFocus();

				//unblock blocked signals after assigning the answer
				TraceList::const_iterator bi = blksig.begin(), be = blksig.end();
				if (bi != be)
				{
					rcp->blockSignals( bi->second);
					++bi;
				}
				foreach (QWidget* cld, rcp->findChildren<QWidget*>())
				{
					if (bi == be) break;
					if (bi->first == cld->objectName())
					{
						qDebug() << "unblock signals of" << cld->metaObject()->className() << cld->objectName();
						cld->blockSignals( bi->second);
						++bi;
					}
				}
			}
		}
	}
	signalPushButtonEnablers();
}

void FormWidget::gotError( const QString& tag_, const QByteArray& data_)
{
	qDebug( ) << "Error for form" << m_form << "and tag" << tag_;

// hand-written plugin, custom request, pass it back directly, don't go over
// generic widget answer part (TODO: there should be a registry map here perhaps)
	FormPluginInterface *plugin = formPlugin( FormCall::name( m_form ) );
	if( plugin ) {
		plugin->gotError( tag_, data_ );
		return;
	}

	WidgetVisitor visitor( m_ui);
	WidgetMessageDispatcher dispatcher( visitor);
	WidgetRequest rq( tag_);

	if (rq.type() == WidgetRequest::Action)
	{
		QList<QWidget*> rcpl = dispatcher.findRecipients( rq.recipientid());
		if (!rcpl.isEmpty())
		{
			qDebug() << "got error tag=" << tag_ << "data=" << data_;
			emit error( QString( data_));

			foreach (QWidget* actionwidget, rcpl)
			{
				QPushButton* button = qobject_cast<QPushButton*>( actionwidget);
				if (button) button->setDown( false);
			}
		}
	}
}

void FormWidget::closeEvent( QCloseEvent *e )
{
	emit closed( );
	e->accept( );
}
