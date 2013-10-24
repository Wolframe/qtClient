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

#include "MainWindow.hpp"
#include "FileFormLoader.hpp"
#include "FileDataLoader.hpp"
#include "NetworkFormLoader.hpp"
#include "NetworkDataLoader.hpp"
#include "settings.hpp"
#include "PreferencesDialog.hpp"
#include "global.hpp"
#include "FormChooseDialog.hpp"
#include "manageServerDefsDialog.hpp"
#include "FormCall.hpp"
#include "debugview/DebugMessageHandler.hpp"
#include <QtGui>
#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QtAlgorithms>
#include <QMessageBox>
#include <QDebug>
#include <QMainWindow>
#include <QMenuBar>
#include <QBuffer>
#include <QList>
#include <QAction>

#ifdef _WIN32
#define WIN32_MEAN_AND_LEAN
#include <windows.h>
#undef max
#endif

#include <algorithm>

// built-in defaults
MainWindow::MainWindow( QWidget *_parent ) : SkeletonMainWindow( _parent ),
	m_formWidget( 0 ), m_uiLoader( 0 ), m_formLoader( 0 ),
	m_dataLoader( 0 ), m_settings( ),
	m_languages( ), m_language( ),
	m_mdiArea( 0 ), m_subWinGroup( 0 ),
	m_terminating( false ), m_debugTerminal( 0 ), m_debugTerminalAction( 0 ),
	m_modalDialog( 0 ), m_modalDialogClosed( true ), m_menuSignalMapper( 0 )
{
	setDebugLogTree( &m_logtree);

// read parameters, first and only one is the optional configurartion files
// containint the settings
	parseArgs( );

// settings override built-in defaults
	readSettings( );

// enable login remember mechanism
	setRememberLogin( true );

// signal mapper for menu actions to load form calls with parameter
	m_menuSignalMapper = new QSignalMapper( this );

	connect( m_menuSignalMapper, SIGNAL( mapped( QObject * ) ),
		this, SLOT( loadMenuForm( QObject * ) ) );
}

void MainWindow::loadMenuForm( QObject *obj )
{
	MenuEntry *menuEntry = qobject_cast<MenuEntry *>( obj );

	qDebug( ) << "MENU: form call" << menuEntry;

	if( settings.mdi ) {
		if( menuEntry->singleton ) {
			foreach( QMdiSubWindow *w, m_mdiArea->subWindowList( ) ) {
				FormWidget *f = qobject_cast<FormWidget *>( w->widget( ) );
				// found a window with the same form, so bring it to the front
				if( menuEntry->form == FormCall::name( f->formCall( ) ) ) {
					m_mdiArea->setActiveSubWindow( w );
					return;
				}
			}
		}
		// no existing window
		(void)CreateMdiSubWindow( menuEntry->form, true );
	} else {
		loadForm( menuEntry->form );
	}
}

void MainWindow::initializeUi( )
{
	m_ui = new Ui::MainWindow( );
	static_cast<Ui::MainWindow *>( m_ui )->setupUi( this );

// add the menu entries for the developer mode
	if( settings.developEnabled ) {
		addDeveloperMenu( );
	}
}

void MainWindow::deleteUi( )
{
	delete static_cast<Ui::MainWindow *>( m_ui );
}

void MainWindow::retranslateUi( )
{
	static_cast<Ui::MainWindow *>( m_ui )->retranslateUi( this );
}

void MainWindow::readSettings( )
{
	if( m_settings.isNull( ) ) {
// read from standard config location (.config/Wolframe or registry)
		settings.read( ORGANIZATION_NAME, APPLICATION_NAME );
	} else {
// read from configuration file given as '-s xxx.conf' on the command line
		settings.read( m_settings );
	}

	setDebugMode( settings.debug);

	if( settings.locale == SYSTEM_LANGUAGE ) {
		m_language = QLocale::system( ).name( );
	} else {
		m_language = settings.locale;
	}

	m_serverDefs = settings.connectionParams;
	m_defaultServer = settings.defaultServer;

// set remember username and connection for the login dialog
	if( settings.saveUsername ) {
		setLastUsername( settings.lastUsername );
		setLastConnName( settings.lastConnection );
	}
}


MainWindow::~MainWindow( )
{
	if( settings.mdi ) {
		if( m_mdiArea )
			m_mdiArea->closeAllSubWindows( );
	} else {
		if( m_formWidget ) {
			delete m_formWidget;
			m_formWidget = 0;
		}
	}
	if( m_debugTerminal ) {
		delete m_debugTerminal;
		m_debugTerminal = 0;
		setDebugLogTree( 0);
	}
	if( m_formLoader ) {
		delete m_formLoader;
		m_formLoader = 0;
	}
	if( m_dataLoader ) {
		delete m_dataLoader;
		m_dataLoader = 0;
	}
	if( m_uiLoader ) {
		delete m_uiLoader;
		m_uiLoader = 0;
	}
}

// --- command line argument handling

void MainWindow::parseArgs( )
{
	QStringList args = QApplication::arguments( );
	if( args.size( ) > 1 ) {
		m_settings = args[1];
	}
}

// -- initialization

void MainWindow::create( )
{
	SkeletonMainWindow::create( );

	setDebugLogTree( &m_logtree);

// install custom output handler (mainly for Unix debugging)
#if QT_VERSION >= 0x050000
	qInstallMessageHandler( &wolframeMessageOutput );
#else
	qInstallMsgHandler( &wolframeMessageOutput );
#endif

// a Qt UI loader for the main theme window and also used by all form widgets
	m_uiLoader = new QUiLoader( );
	m_uiLoader->setLanguageChangeEnabled( true );
// for Windows, to find plugins in local installation directory
	m_uiLoader->addPluginPath( "." );
// for running in git workspace (for development)
	m_uiLoader->addPluginPath( "../plugins/filechooser" );
	m_uiLoader->addPluginPath( "../plugins/picturechooser" );
	QStringList paths = m_uiLoader->pluginPaths( );
	qDebug( ) << "Will load custom widget plugins from" << paths;

// for testing, load lists of available forms from the files system or
// a local sqlite database, pass the form loader to the FormWidget
	switch( settings.uiLoadMode ) {
		case LoadMode::FILE:
			m_formLoader = new FileFormLoader( settings.uiFormsDir, settings.uiFormTranslationsDir, settings.uiFormResourcesDir, settings.uiMenusDir );
			break;

		case LoadMode::NETWORK:
			// skip, delay, needs a working connection for this
			break;

		case LoadMode::UNDEFINED:
			QMessageBox::critical( this, tr( "Configuration error" ), "Unknown value for UI load mode in the configuration", QMessageBox::Ok );
			break;
	}

// ..same for the data loader
	switch( settings.dataLoadMode ) {
		case LoadMode::FILE:
			m_dataLoader = new FileDataLoader( settings.dataLoaderDir );
			break;

		case LoadMode::NETWORK:
			// skip, delay, needs a working connection for this
			break;

		case LoadMode::UNDEFINED:
			QMessageBox::critical( this, tr( "Configuration error" ), "Unknown value for data load mode in the configuration", QMessageBox::Ok );
			break;
	}

// link the form loader for form loader notifications needed by the main window
// (list of forms for form menu, list of language for language picker)
	connect( m_formLoader, SIGNAL( languageCodesLoaded( QStringList ) ),
		this, SLOT( languageCodesLoaded( QStringList ) ) );
	connect( m_formLoader, SIGNAL( formListLoaded( QStringList ) ),
		this, SLOT( formListLoaded( QStringList ) ) );
	connect( m_formLoader, SIGNAL( menuListLoaded( QStringList ) ),
		this, SLOT( menuListLoaded( QStringList ) ) );

// create central widget, either as MDI area or as one form widget
	if( settings.mdi ) {
		m_mdiArea = findChild<QMdiArea *>( "centralWidget" );
		if( m_mdiArea ) {
			m_mdiArea->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
			m_mdiArea->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
			m_mdiArea->setActivationOrder( QMdiArea::ActivationHistoryOrder );

// attach mdi area to some specific signals
			connect( m_mdiArea, SIGNAL( subWindowActivated( QMdiSubWindow * ) ),
				this, SLOT( subWindowChanged( QMdiSubWindow * ) ) );

// connect some MDI specific signals to menu and toolbar
			QAction *action = findChild<QAction *>( "actionTile" );
			if( action ) {
				action->setEnabled( true );
				connect( action, SIGNAL( triggered( ) ),
					m_mdiArea, SLOT( tileSubWindows( ) ) );
			}
			action = findChild<QAction *>( "actionCascade" );
			if( action ) {
				action->setEnabled( true );
				connect( action, SIGNAL( triggered( ) ),
					m_mdiArea, SLOT( cascadeSubWindows( ) ) );
			}
		} else {
// missing a MDI area, so we disable the m_mdi flag
			settings.mdi = false;
		}
	}

// in local file UI and data mode we can load the form right away, we don't
// wait for the user to log in
	if( settings.uiLoadMode == LoadMode::FILE && settings.dataLoadMode == LoadMode::FILE ) {
		restoreStateAndPositions( );
	}

// update shortcuts to standard ones
	updateActionShortcuts( );

// load language codes for language picker
	loadLanguages( );

// load language resources, repaints the whole interface if necessary
	loadLanguage( m_language );
}

void MainWindow::CreateFormWidget( const QString &name )
{
	m_formWidget = new FormWidget( m_formLoader, m_dataLoader, &m_globals, m_uiLoader, this, settings.debug, settings.uiFormsDir, m_wolframeClient, settings.mdi );

	connect( m_formWidget, SIGNAL( formLoaded( QString ) ),
		this, SLOT( formLoaded( QString ) ) );
	connect( m_formWidget, SIGNAL( formModal( QString ) ),
		this, SLOT( formModal( QString ) ) );
	connect( m_formWidget, SIGNAL( formNewWindow( QString ) ),
		this, SLOT( formNewWindow( QString ) ) );
	connect( m_formWidget, SIGNAL( error( QString ) ),
		this, SLOT( formError( QString ) ) );

	setCentralWidget( m_formWidget );
	m_formWidget->setLanguage( m_language );

	loadForm( name );
}

void MainWindow::activateAction( const QString name, bool enabled )
{
	QAction *action = findChild<QAction *>( name );
	if( action ) {
		action->setEnabled( enabled );
	}
}

QKeySequence::StandardKey MainWindow::defaultKeySequenceFromString( const QString s )
{
	if( s == "QKeySequence::NextChild" ) {
		return QKeySequence::NextChild;
	} else if( s == "QKeySequence::PreviousChild" ) {
		return QKeySequence::PreviousChild;
	} else if( s == "QKeySequence::Print" ) {
		return QKeySequence::Print;
	} else if( s == "QKeySequence::Quit" ) {
		return QKeySequence::Quit;
	} else if( s == "QKeySequence::Undo" ) {
		return QKeySequence::Undo;
	} else if( s == "QKeySequence::Redo" ) {
		return QKeySequence::Redo;
	} else if( s == "QKeySequence::Cut" ) {
		return QKeySequence::Cut;
	} else if( s == "QKeySequence::Copy" ) {
		return QKeySequence::Copy;
	} else if( s == "QKeySequence::Paste" ) {
		return QKeySequence::Paste;
	} else if( s == "QKeySequence::Delete" ) {
		return QKeySequence::Delete;
	} else if( s == "QKeySequence::SelectAll" ) {
		return QKeySequence::SelectAll;
	} else if( s == "QKeySequence::Refresh" ) {
		return QKeySequence::Refresh;
	} else if( s == "QKeySequence::Open" ) {
		return QKeySequence::Open;
	} else if( s == "QKeySequence::New" ) {
		return QKeySequence::New;
	} else if( s == "QKeySequence::Close" ) {
		return QKeySequence::Close;
	} else if( s == "QKeySequence::Preferences" ) {
		return QKeySequence::Preferences;
	} else if( s == "QKeySequence::HelpContents" ) {
		return QKeySequence::HelpContents;
	} else {
		return QKeySequence::UnknownKey;
	}
}

void MainWindow::updateActionShortcuts( )
{
	foreach( QAction *action, findChildren<QAction *>( ) ) {
		QString s = action->property( "defaultShortcut" ).toString();
		if( !s.isEmpty( ) ) {
			QKeySequence::StandardKey shortcut = defaultKeySequenceFromString( s );
			if( shortcut != QKeySequence::UnknownKey ) {
				QKeySequence seq( shortcut );
				if( !seq.isEmpty( ) ) {
					action->setShortcuts( defaultKeySequenceFromString( s ) );
					qDebug( ) << "ACTION" << action << "gets default shortcut" << s;
				} else {
					qDebug( ) << "ACTION" << action << "keeps shortcuts from ui resource" << action->shortcuts( );
				}
			}
		}
	}
}

// --- handling protocol changes (connection states and errors)

void MainWindow::disconnected( )
{
	if( m_debugTerminal ) {
		m_debugTerminalAction->setChecked( false );
		m_debugTerminal->deleteLater( );
		m_debugTerminal = 0;
		setDebugLogTree( 0);
	}

	if( settings.uiLoadMode == LoadMode::NETWORK ) {
		delete m_uiLoader;
		m_uiLoader = 0;
	}

	if( settings.dataLoadMode == LoadMode::NETWORK ) {
		delete m_dataLoader;
		m_dataLoader = 0;
	}

	if( m_terminating ) {
		close( );
	}

	SkeletonMainWindow::disconnected( );
}

void MainWindow::authOk( )
{
	SkeletonMainWindow::authOk( );

// create network based form ...
	if( settings.uiLoadMode == LoadMode::NETWORK ) {
		m_formLoader = new NetworkFormLoader( m_wolframeClient );
	}

// ...and data loaders
	if( settings.dataLoadMode == LoadMode::NETWORK ) {
		m_dataLoader = new NetworkDataLoader( m_wolframeClient, settings.debug );
	}

	restoreStateAndPositions( );
	updateMenusAndToolbars( );

// initialize a form list load in order to get list of menus to be
// shown in the main menu and the list of forms in the developer menues
	if( m_formLoader ) {
		m_formLoader->initiateListLoad( );
		m_formLoader->initiateMenuListLoad( );
	}
}

void MainWindow::loadLanguages( )
{
// get the list of available languages in the forms
	if( m_formLoader )
		m_formLoader->initiateGetLanguageCodes( );
}

void MainWindow::languageCodesLoaded( QStringList languages )
{
	QStringList languageCodes;
	languageCodes.push_back( DEFAULT_LOCALE ); // default locale, always around

// read list of supported languages based on the wolfclient translations
	QDir translationDir( "i18n" );
	translationDir.setFilter( QDir::Files | QDir::NoDotAndDotDot );
	translationDir.setSorting( QDir::Name );
	QStringList filters;
	filters << "*.qm";
	translationDir.setNameFilters( filters );
	QStringList localeFiles = translationDir.entryList( );
	QMutableStringListIterator it( localeFiles );
	while( it.hasNext( ) ) {
		it.next( );
		QStringList parts = it.value( ).split( "." );
		languageCodes.push_back( parts[1] );
	}

// add the ones supported in forms
	languageCodes.append( languages );
	languageCodes.removeDuplicates( );

// remember languages for preferences dialog
	m_languages = languageCodes;

// does the menu exist?
	QMenu *languageMenu = this->findChild<QMenu *>( "menuLanguages" );
	if( !languageMenu ) return;

// construct a menu showing all languages
	languageMenu->clear( );
	QActionGroup *languageGroup = new QActionGroup( languageMenu );
	languageGroup->setExclusive( true );
	foreach( QString language, m_languages ) {
		QLocale myLocale( language );
		QAction *action = new QAction( myLocale.languageToString( myLocale.language( ) ) + " (" + language + ")", languageGroup );
		action->setCheckable( true );
		action->setData( QVariant( language ) );
		languageGroup->addAction( action );
		if( language == m_language ) action->setChecked( true );
	}
	languageMenu->addActions( languageGroup->actions( ) );
	connect( languageGroup, SIGNAL( triggered( QAction * ) ), this, SLOT( languageSelected( QAction * ) ) );
}

void MainWindow::languageSelected( QAction *action )
{
	QString language = action->data( ).toString( );
	if( language != m_language )
		loadLanguage( language );
}

void MainWindow::switchTranslator( QTranslator &translator, const QString &filename, const QString &i18n )
{
	qApp->removeTranslator( &translator );

	if( translator.load( filename, i18n ) ) {
		qApp->installTranslator( &translator );
	} else {
		qWarning( ) << "Error while loading translations for" << filename << "from directory" << i18n;
	}
}

void MainWindow::loadLanguage( QString language )
{
	qDebug( ) << "Switching interface language to " << language;

// change language on global level
	switchTranslator( m_translatorApp, QString( "wolfclient.%1.qm" ).arg( language ), "i18n" );
	switchTranslator( m_translatorQt, QString( "qt_%1.qm" ).arg( language ), "/usr/share/qt/translations/" );

// also set language of the form widget(s)
	if( settings.mdi ) {
		foreach( QMdiSubWindow *w, m_mdiArea->subWindowList( ) ) {
			FormWidget *f = qobject_cast<FormWidget *>( w->widget( ) );
			f->setLanguage( language );
			f->reload( );
		}
	} else {
		if( m_formWidget ) {
			m_formWidget->setLanguage( language );
			m_formWidget->reload( );
		}
	}

	m_language = language;
}

void MainWindow::changeEvent( QEvent* _event )
{
	if( _event )	{
		if ( _event->type() == QEvent::LanguageChange )
			retranslateUi( );
		else if ( _event->type() == QEvent::LocaleChange )	{
			QString locale = QLocale::system( ).name( );
			locale.truncate( locale.lastIndexOf( '_' ) );
			loadLanguage( locale );
		}
	}

	QMainWindow::changeEvent( _event );
}

void MainWindow::menuLoaded( QString name, QByteArray menu )
{
	qDebug( ) << "MENU: checking for main menu" << name;
	QBuffer buf( &menu );
	QWidget *ui = m_uiLoader->load( &buf, 0 );

// read the UI and glue the menu into the main menu bar
// TODO: a menu can be edited only in a QMainWindow for now
	QMainWindow *w = qobject_cast<QMainWindow *>( ui );
	if( w ) {
		QMenuBar *bar = qobject_cast<QMenuBar *>( w->menuWidget( ) );
		if( bar ) {
			qDebug( ) << "MENU:" << name << "is a menu, integrating it into menu bar";
			QList<QMenu *> menus = bar->findChildren<QMenu *>( );
			foreach( QMenu *menu, menus ) {
				addGlueMenuAction( menu );
				QString beforeMenuName = bar->property( "beforeMenu" ).toString( );
				QAction *glueAction = 0;
				if( beforeMenuName.isEmpty( ) ) {
					qDebug( ) << "MENU: glueing menu" << menu->title( );
					glueAction = menuBar( )->addMenu( menu );
				} else {
					QList<QMenu *> existingMenus = menuBar( )->findChildren<QMenu *>( );
					foreach( QMenu *m, existingMenus ) {
						if( m->objectName( ) == beforeMenuName ) {
							qDebug( ) << "MENU: glueing menu" << menu->title( ) << "before main menu entry" << m->objectName( );
							glueAction = menuBar( )->insertMenu( m->menuAction( ), menu );
						}
					}
					if( !glueAction ) {
						qWarning( ) << "MENU: beforeMenu" << beforeMenuName << "missing in MainWindow.ui";
						glueAction = menuBar( )->addMenu( menu );
					}
				}
				if( glueAction ) {
					m_actions.push_back( glueAction );
				}
			}
		}
	}
}

void MainWindow::addGlueMenuAction( QMenu *menu )
{
// read dynamic property 'form' and connect it to loadForm calls
	foreach( QAction *action, menu->actions( ) ) {
		QString form = action->property( "form" ).toString( );
		if( !form.isEmpty( ) ) {
			qDebug( ) << "MENU: linking action" << action->text( ) << "to form call" << form;
			connect( action, SIGNAL( triggered( ) ),
				m_menuSignalMapper, SLOT( map( ) ) );
			MenuEntry *menuEntry = new MenuEntry( form, action->property( "singletonWindow" ).toBool( ) );
			m_menuSignalMapper->setMapping( action, menuEntry );
		}
	}
}

void MainWindow::formListLoaded( QStringList forms )
{
	m_forms = forms;
}

void MainWindow::menuListLoaded( QStringList menus )
{
	connect( m_formLoader, SIGNAL( menuLoaded( QString, QByteArray ) ),
		this, SLOT( menuLoaded( QString, QByteArray ) ) );

// iterate over all UIs and find out which ones are QMainWindows (for
// now the placeholder for all menues)
	foreach( QString menu, menus ) {
		m_formLoader->initiateMenuLoad( menu );
	}

	disconnect( m_formLoader, SIGNAL( menuLoaded( QString, QByteArray ) ),
		this, SLOT( menuLoaded( QString, QByteArray ) ) );
}

void MainWindow::loadForm( QString name, const bool newWindow )
{
	setWaitState( true );
	
// delegate form loading to form widget
	if( m_formWidget )
		m_formWidget->loadForm( name, false, newWindow );
}

void MainWindow::endModal( )
{
	if( m_modalDialogClosed ) return;

	m_modalDialogClosed = true;

	qDebug( ) << "endModal";

// restore wiring in main frame
	connect( m_formWidget, SIGNAL( formLoaded( QString ) ),
		this, SLOT( formLoaded( QString ) ) );
	connect( m_formWidget, SIGNAL( formModal( QString ) ),
		this, SLOT( formModal( QString ) ) );
	connect( m_formWidget, SIGNAL( formNewWindow( QString ) ),
		this, SLOT( formNewWindow( QString ) ) );
	connect( m_formWidget, SIGNAL( error( QString ) ),
		this, SLOT( formError( QString ) ) );
	connect( m_formWidget,SIGNAL( destroyed( ) ),
		this, SLOT( updateMenusAndToolbars( ) ) );

// this triggers endModal a second time!
	m_modalDialog->close( );
	m_modalDialog->deleteLater( );
}

void MainWindow::formNewWindow( QString name )
{
// open new MDI subwindow in MDI case (default, singletonWindow avoids this)
	if( settings.mdi ) {
		(void)CreateMdiSubWindow( name, true, true );
// kiosk mode, always load form normally in the only form widget there is
	} else {
		loadForm( name );
	}
}

void MainWindow::formModal( QString name )
{
	m_modalDialog = new QDialog( this );

	FormWidget *formWidget = new FormWidget( m_formLoader, m_dataLoader, &m_globals, m_uiLoader, m_modalDialog, settings.debug, settings.uiFormsDir, m_wolframeClient, settings.mdi );

	connect( formWidget, SIGNAL( formLoaded( QString ) ),
		this, SLOT( formLoaded( QString ) ) );
	connect( formWidget, SIGNAL( formModal( QString ) ),
		this, SLOT( formModal( QString ) ) );
	connect( m_formWidget, SIGNAL( formNewWindow( QString ) ),
		this, SLOT( formNewWindow( QString ) ) );
	connect( formWidget, SIGNAL( error( QString ) ),
		this, SLOT( formError( QString ) ) );
	connect( formWidget,SIGNAL( destroyed( ) ),
		this, SLOT( updateMenusAndToolbars( ) ) );
	connect( formWidget,SIGNAL( closed( ) ),
		this, SLOT( endModal( ) ) );

// we are modal, so temporaily we have to disconnect the parent form from
// the main window in order not to trigger funny results
	disconnect( m_formWidget, SIGNAL( formLoaded( QString ) ), 0, 0 );
	disconnect( m_formWidget, SIGNAL( formModal( QString ) ), 0, 0 );
	disconnect( m_formWidget, SIGNAL( formNewWindow( QString ) ), 0, 0 );
	disconnect( m_formWidget, SIGNAL( error( QString ) ), 0, 0 );
	disconnect( m_formWidget, SIGNAL( destroyed( ) ), 0, 0 );

	QVBoxLayout *l = new QVBoxLayout( m_modalDialog );
	l->addWidget( formWidget );

	formWidget->setLanguage( m_language );
	formWidget->loadForm( name, true );

	connect( m_modalDialog, SIGNAL( rejected( ) ),
		this, SLOT( endModal( ) ) );

	m_modalDialogClosed = false;

	m_modalDialog->show( );
}

void MainWindow::formLoaded( QString /*name*/ )
{
	setWaitState( false );
	
// in MDI mode update the title of the sub window, otherwise update window title
	if( settings.mdi ) {
		QMdiSubWindow *mdiSubWindow = m_mdiArea->activeSubWindow( );
		if( mdiSubWindow ) {
			QString title = m_formWidget->windowTitle( );
			mdiSubWindow->setWindowTitle( title );

			QIcon icon = m_formWidget->getWindowIcon( );
			if( !icon.isNull( ) ) {
				qDebug( ) << "Setting window icon" << m_formWidget;
				mdiSubWindow->setWindowIcon( icon );
			} else {
				qDebug( ) << "Setting application icon";
				mdiSubWindow->setWindowIcon( windowIcon( ) );
			}

			m_mdiArea->update( );

			QAction *action = m_revSubWinMap.value( mdiSubWindow );
			if( action ) {
				int idx = action->data( ).toInt( );
				QString text = composeWindowListTitle( idx, title );
				action->setText( text );
			}
		}
	} else {
		setWindowTitle( tr( "wolfclient - %1" ).arg( m_formWidget->windowTitle( ) ) );
	}
}

QString MainWindow::composeWindowListTitle( const int idx, const QString title )
{
	QString text;

	if( idx < 10 ) {
		text = tr( "&%1 %2" ).arg( idx ).arg( title );
	} else {
		text = tr( "%1 %2" ).arg( idx ).arg( title );
	}

	return text;
}

void MainWindow::updateWindowMenu( )
{
	QMdiSubWindow *mdiSubWindow = m_mdiArea->activeSubWindow( );
	QAction *action = m_revSubWinMap.value( mdiSubWindow );
	if( action )
		action->setChecked( true );
}

void MainWindow::formError( QString error )
{
	setWaitState( false );
	
	QMessageBox::critical( this, tr( "Form error" ), error, QMessageBox::Ok );
}

void MainWindow::on_actionExit_triggered( )
{
	m_terminating = true;

	if( settings.uiLoadMode == LoadMode::NETWORK || settings.dataLoadMode == LoadMode::NETWORK ) {
		if( m_wolframeClient ) {
			logout( );
		} else {
			close( );
		}
	} else {
// terminate brutally in local mode (this is for a connection from the debug window)
		if( m_wolframeClient )
			disconnect( m_wolframeClient, SIGNAL( error( QString ) ), 0, 0 );

		if( settings.uiLoadMode == LoadMode::FILE && settings.dataLoadMode == LoadMode::FILE ) {
			storeStateAndPositions( );
		}

		close( );
	}

	storeSettings( );
}

void MainWindow::restoreStateAndPositions( )
{
// restore main window position and size
	move( settings.mainWindowPos );
	resize( settings.mainWindowSize );

// load initial form, load forms and position of windows from settings,
// of none there, load init form in a MDI subwindow or directly
	if( settings.mdi ) {
		if( settings.saveRestoreState ) {
			for( int i = 0; i < settings.states.size( ); i++ ) {
				WinState state = settings.states[i];
				// all stored windows had newWindow set, hence true
				QMdiSubWindow *w = CreateMdiSubWindow( state.form, true );
				w->move( state.position );
				w->resize( state.size );
				FormWidget *f = qobject_cast<FormWidget *>( w->widget( ) );
				f->setWidgetStates( state.widgetStates );
			}
			foreach( QMdiSubWindow *w, m_mdiArea->subWindowList( ) ) {
				FormWidget *f = qobject_cast<FormWidget *>( w->widget( ) );
				if( f->formCall( ) == settings.focusedWindow ) {
					m_mdiArea->setActiveSubWindow( w );
				}
			}
		} else {
			(void)CreateMdiSubWindow( "init", true );
		}
	} else {
		if( settings.saveRestoreState && settings.states.size( ) > 0 ) {
			WinState state = settings.states[0];
			CreateFormWidget( state.form );
		} else {
			CreateFormWidget( "init" );
		}
	}
}

void MainWindow::storeStateAndPositions( )
{
// save our own size and position
	if( settings.saveRestoreState ) {
		settings.mainWindowPos = pos( );
		settings.mainWindowSize = size( );
	}

// save position/size and state of subwindows (if wished)
	if( settings.saveRestoreState ) {
		settings.states.clear( );
		if( settings.mdi ) {
			foreach( QMdiSubWindow *w, m_mdiArea->subWindowList( ) ) {
				WinState state;
				FormWidget *f = qobject_cast<FormWidget *>( w->widget( ) );
				if( w == m_mdiArea->activeSubWindow( ) ) {
					settings.focusedWindow = f->formCall( );
				}
				state.form = f->formCall( );
				state.position = w->pos( );
				state.size = w->size( );
				state.widgetStates = f->getWidgetStates( );
				settings.states.append( state );
			}
		} else {
			if( m_formWidget ) {
				WinState state;
				state.form = m_formWidget->formCall( );
				state.position = m_formWidget->pos( );
				state.size = m_formWidget->size( );
				state.widgetStates = m_formWidget->getWidgetStates( );
				settings.states.append( state );
			}
		}
	}
}

void MainWindow::storeSettings( )
{
// connection parameters
	settings.connectionParams = m_serverDefs;
	settings.defaultServer = m_defaultServer;

// optionally remember last connection and username
	if( settings.saveUsername ) {
		settings.lastUsername = lastUsername( );
		settings.lastConnection = lastConnName( );
	}

// store them to the configuration file or the default configuration
// file respectively registry
	if( m_settings.isEmpty( ) ) {
		settings.write( ORGANIZATION_NAME, APPLICATION_NAME );
	} else {
		settings.write( m_settings );
	}
}

void MainWindow::closeEvent( QCloseEvent *e )
{
	storeSettings( );

	e->accept( );
}

void MainWindow::on_actionPreferences_triggered( )
{
	PreferencesDialog prefs( settings, m_languages, this );
	if( prefs.exec( ) == QDialog::Accepted ) {
		qDebug( ) << "Reloading application";
		storeSettings( );
		QApplication::instance( )->exit( RESTART_CODE );
	}
}

void MainWindow::on_actionAbout_triggered( )
{
	QMessageBox aboutBox( this );
	aboutBox.setParent( this );
	QString info = QString(
		tr( "wolfclient\n\nVersion %1\n\n(c) 2011 - 2013 Wolframe Group" )
			.arg( WOLFRAME_VERSION ) );
	aboutBox.setText( info );
	aboutBox.setIconPixmap( QPixmap( QString( ":/images/wolfClient.png" ) ) );
	aboutBox.exec( );
}

void MainWindow::on_actionAboutQt_triggered( )
{
	QMessageBox::aboutQt( this, tr( "wolfclient" ) );
}

// -- form handling

void MainWindow::on_actionReload_triggered( )
{
	m_formWidget->reload( );
}

// -- MDI mode

QMdiSubWindow *MainWindow::CreateMdiSubWindow( const QString &form, const bool newWindow, const bool openAtCursorPosition )
{
	FormWidget *formWidget = new FormWidget( m_formLoader, m_dataLoader, &m_globals, m_uiLoader, this, settings.debug, settings.uiFormsDir, m_wolframeClient, settings.mdi );

	connect( formWidget, SIGNAL( formLoaded( QString ) ),
		this, SLOT( formLoaded( QString ) ) );
	connect( formWidget, SIGNAL( formModal( QString ) ),
		this, SLOT( formModal( QString ) ) );
	connect( formWidget, SIGNAL( formNewWindow( QString ) ),
		this, SLOT( formNewWindow( QString ) ) );
	connect( formWidget, SIGNAL( error( QString ) ),
		this, SLOT( formError( QString ) ) );
	connect( formWidget, SIGNAL( destroyed( ) ),
		this, SLOT( updateMenusAndToolbars( ) ) );

	QMdiSubWindow *mdiSubWindow = m_mdiArea->addSubWindow( formWidget );
	mdiSubWindow->setAttribute( Qt::WA_DeleteOnClose );

	connect( formWidget, SIGNAL( closed( ) ),
		mdiSubWindow, SLOT( close( ) ) );

	m_formWidget = formWidget; // ugly dirty hack, must ammend later
	formWidget->show( );
	formWidget->setLanguage( m_language );
	loadForm( form, newWindow );

	mdiSubWindow->resize( mdiSubWindow->sizeHint( ) );

// do some move and resize tricks to make the new MDI window appear
// completly visible and as close to the user cursor as possible
	if( openAtCursorPosition ) {
		QPoint pos = m_mdiArea->mapFromGlobal( QCursor::pos( ) );
		QSize size = mdiSubWindow->widget( )->size( );
		QSize areaSize = m_mdiArea->size( );
		if( pos.y( ) + size.height( ) > areaSize.height( ) ) {
			pos.setY( std::max( 0, areaSize.height( ) - size.height( ) - 50 ) );
		}
		if( pos.x( ) + size.width( ) > areaSize.width( ) ) {
			pos.setX( std::max( 0, areaSize.width( ) - size.width( ) ) );
		}
		mdiSubWindow->move( pos );
	}

	updateMenusAndToolbars( );

	return mdiSubWindow;
}

void MainWindow::subWindowSelected( QAction *action )
{
	QMdiSubWindow *w = m_subWinMap.value( action );
	m_mdiArea->setActiveSubWindow( w );
}

void MainWindow::subWindowChanged( QMdiSubWindow *w )
{
	if( !w ) return;

	m_formWidget = qobject_cast<FormWidget *>( w->widget( ) );

	updateWindowMenu( );
}

void MainWindow::on_actionNextWindow_triggered( )
{
	m_mdiArea->activateNextSubWindow( );

	updateWindowMenu( );
}

void MainWindow::on_actionPreviousWindow_triggered( )
{
	m_mdiArea->activatePreviousSubWindow( );

	updateWindowMenu( );
}

void MainWindow::on_actionClose_triggered( )
{
	m_mdiArea->closeActiveSubWindow( );

	updateMenusAndToolbars( );
}

void MainWindow::on_actionCloseAll_triggered( )
{
	m_mdiArea->closeAllSubWindows( );

	updateMenusAndToolbars( );
}

int MainWindow::nofSubWindows( ) const
{
	QList<QMdiSubWindow *> list = m_mdiArea->subWindowList( );
	return list.count( );
}

void MainWindow::updateMdiMenusAndToolbars( )
{
	if( !m_mdiArea ) return;

// present new form menu entry if logged in
	activateAction( "actionOpenFormNewWindow",
		( settings.uiLoadMode == LoadMode::FILE && settings.dataLoadMode == LoadMode::FILE ) ||
		m_wolframeClient );

// enable/disable menu/toolbar items depending on the number of subwindows
	activateAction( "actionClose", nofSubWindows( ) > 0 );
	activateAction( "actionCloseAll", nofSubWindows( ) > 0 );
	activateAction( "actionNextWindow", nofSubWindows( ) > 1 );
	activateAction( "actionPreviousWindow", nofSubWindows( ) > 1 );
	activateAction( "actionTile", nofSubWindows( ) > 0 );
	activateAction( "actionCascade", nofSubWindows( ) > 0 );

// recreate the subwindow menu and mark the currently selected submenu
	QMenu *windowMenu = this->findChild<QMenu *>( "menuWindow" );
	if( windowMenu ) {
		m_subWinMap.clear( );
		m_revSubWinMap.clear( );

		if( m_subWinGroup ) {
			foreach( QAction *action, m_subWinGroup->actions( ) ) {
				m_subWinGroup->removeAction( action );
				delete action;
			}
		}

		m_subWinGroup = new QActionGroup( windowMenu );
		m_subWinGroup->setExclusive( true );
		QAction *action = new QAction( "", m_subWinGroup );
		action->setSeparator( true );
		m_subWinGroup->addAction( action );
		int idx = 1;
		foreach( QMdiSubWindow *w, m_mdiArea->subWindowList( ) ) {
			// TODO: translation is tricky here: the form widget has
			// a QTranslator class and can call translate on it using
			// the translation resource of the corresponding form!
			// see QString QTranslator::translate ( const char * context, const char * sourceText, const char * disambiguation, int n ) const
			QString title = w->windowTitle( );
			QString text = composeWindowListTitle( idx, title );
			action = new QAction( text, m_subWinGroup );
			action->setCheckable( true );
			action->setData( QVariant( idx ) );
			m_subWinGroup->addAction( action );
			if( w == m_mdiArea->activeSubWindow( ) ) {
				action->setChecked( true );
			}
			m_subWinMap.insert( action, w );
			m_revSubWinMap.insert( w, action );
			idx++;
		}
		windowMenu->addActions( m_subWinGroup->actions( ) );

		connect( m_subWinGroup, SIGNAL( triggered( QAction * ) ),
			this, SLOT( subWindowSelected( QAction * ) ) );
	}
}

void MainWindow::updateMenusAndToolbars( )
{
	SkeletonMainWindow::updateMenusAndToolbars( );

// logged in or logged out?
	activateAction( "actionOpenForm",
		( ( settings.uiLoadMode == LoadMode::FILE && settings.dataLoadMode == LoadMode::FILE )
		|| ( m_wolframeClient && m_wolframeClient->isConnected( ) ) )
		&& ( !settings.mdi || ( settings.mdi && nofSubWindows( ) > 0 ) ) );
	activateAction( "actionReload",
		( settings.uiLoadMode == LoadMode::FILE && settings.dataLoadMode == LoadMode::FILE ) ||
		( m_wolframeClient && ( !settings.mdi || ( settings.mdi && nofSubWindows( ) > 0 ) ) ) );

	if( settings.uiLoadMode == LoadMode::NETWORK || settings.dataLoadMode == LoadMode::NETWORK ) {
		activateAction( "actionLogin", !m_wolframeClient || !m_wolframeClient->isConnected( ) );
		activateAction( "actionLogout", m_wolframeClient && m_wolframeClient->isConnected( ) );
	}

// MDI menus and toolbars
	if( settings.mdi ) {
		updateMdiMenusAndToolbars( );
	}

// edit menu (TODO: this is provisoric)
	activateAction( "actionUndo", m_wolframeClient && ( !settings.mdi || ( settings.mdi && nofSubWindows( ) > 0 ) ) );
	activateAction( "actionRedo", m_wolframeClient && ( !settings.mdi || ( settings.mdi && nofSubWindows( ) > 0 ) ) );
	activateAction( "actionCut", m_wolframeClient && ( !settings.mdi || ( settings.mdi && nofSubWindows( ) > 0 ) ) );
	activateAction( "actionCopy", m_wolframeClient && ( !settings.mdi || ( settings.mdi && nofSubWindows( ) > 0 ) ) );
	activateAction( "actionPaste", m_wolframeClient && ( !settings.mdi || ( settings.mdi && nofSubWindows( ) > 0 ) ) );
	activateAction( "actionDelete", m_wolframeClient && ( !settings.mdi || ( settings.mdi && nofSubWindows( ) > 0 ) ) );
	activateAction( "actionSelectAll", m_wolframeClient && ( !settings.mdi || ( settings.mdi && nofSubWindows( ) > 0 ) ) );

// developer menu: debug terminal
	if( m_debugTerminalAction )
		m_debugTerminalAction->setEnabled( m_debugTerminal );
}

// -- logins/logouts/connections

void MainWindow::login( )
{
	SkeletonMainWindow::login( );

// create a debug terminal and attach it to the protocol client
	if( settings.debug && settings.developEnabled ) {
		m_debugTerminal = new DebugViewWidget( this );
		m_debugTerminal->setAttribute( Qt::WA_DeleteOnClose );
		setDebugLogTree( &m_logtree);

		qDebug( ) << "Debug window initialized";
	}
}

void MainWindow::removeApplicationMenus( )
{
	foreach( QAction *action, m_actions ) {
		action->setVisible( false );
	}
	m_actions.clear( );
}

void MainWindow::logout( )
{
	removeApplicationMenus( );
	storeStateAndPositions( );
	storeSettings( );

	if( !m_modalDialogClosed ) {
		endModal( );
	}

	if( settings.mdi ) {
		m_mdiArea->closeAllSubWindows( );
		m_formWidget = 0; // because the last mdi window assigned this! See HACK above
	} else {
		m_formWidget->hide( );
		delete m_formWidget;
		m_formWidget = 0;
	}

	SkeletonMainWindow::logout( );
}

void MainWindow::error( QString error )
{
	SkeletonMainWindow::error( error );

	if( !m_wolframeClient->isConnected( ) ) {
		if( !m_modalDialogClosed ) {
			endModal( );
		}
		if( settings.mdi ) {
			m_mdiArea->closeAllSubWindows( );
			m_formWidget = 0; // see above
		} else {
			delete m_formWidget;
			m_formWidget = 0;
		}
		removeApplicationMenus( );
	}
}

// -- developer stuff

void MainWindow::showDebugTerminal( bool checked )
{
	if( m_debugTerminal ) {
		setDebugLogTree( &m_logtree);
		setDebugMode( checked);

		if( checked ) {
			m_debugTerminal->bringToFront( );
		} else {
			m_debugTerminal->hide( );
		}
	}
}

void MainWindow::removeDebugToggle( )
{
	m_debugTerminalAction->setChecked( false );
	m_logtree.clear();
	setDebugLogTree( 0);
	setDebugMode( false);
}

void MainWindow::openForm( )
{
	FormChooseDialog d( m_forms, this );
	if( d.exec( ) == QDialog::Accepted ) {
		QString form = d.form( );
		loadForm( form );
	}
}

void MainWindow::openFormNew( )
{
	FormChooseDialog d( m_forms, this );
	if( d.exec( ) == QDialog::Accepted ) {
		(void)CreateMdiSubWindow( d.form( ), true );
	}
}

void MainWindow::addDeveloperMenu( )
{
	QMenu *developerMenu = menuBar( )->addMenu( tr( "&Developer" ) );

	m_debugTerminalAction = new QAction( QIcon( ":/images/debug.png" ), tr( "&Debugging Terminal..." ), this );
	m_debugTerminalAction->setStatusTip( tr( "Open debug terminal showing the Wolframe protocol" ));
	m_debugTerminalAction->setCheckable( true );
	m_debugTerminalAction->setShortcut( QKeySequence( "Ctrl+Alt+D" ) );
	developerMenu->addAction( m_debugTerminalAction );

	developerMenu->addSeparator( );

	QAction *m_openFormAction = new QAction( tr( "&Open form" ), this );
	m_openFormAction->setObjectName( QString::fromUtf8( "actionOpenForm") );
	m_openFormAction->setStatusTip( tr( "Open form in current window" ) );
	m_openFormAction->setEnabled( false );
	developerMenu->addAction( m_openFormAction );

	QAction *m_openFormNewWindowAction = new QAction( tr( "Open form in &new window" ), this );
	m_openFormNewWindowAction->setStatusTip( tr( "Open form in a new window" ) );
	m_openFormNewWindowAction->setObjectName( QString::fromUtf8( "actionOpenFormNewWindow") );
	m_openFormNewWindowAction->setEnabled( false );
	developerMenu->addAction( m_openFormNewWindowAction );

	QToolBar *developerToolBar = addToolBar( tr( "Developer" ));
	developerToolBar->addAction( m_debugTerminalAction );

	connect( m_debugTerminalAction, SIGNAL( toggled( bool ) ), this,
		SLOT( showDebugTerminal( bool ) ) );
	connect( m_openFormAction, SIGNAL( triggered( ) ), this,
		SLOT( openForm( ) ) );
	connect( m_openFormNewWindowAction, SIGNAL( triggered( ) ), this,
		SLOT( openFormNew( ) ) );
}
