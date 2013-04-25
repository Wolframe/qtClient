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

#include "SkeletonMainWindow.hpp"

#include "WolframeClient.hpp"
#include "ui_SkeletonMainWindow.h"
#include "loginDialog.hpp"
#include "manageServersDialog.hpp"

#include <QMessageBox>
#include <QDebug>
#include <QLabel>

SkeletonMainWindow::SkeletonMainWindow( QWidget *_parent ) : QMainWindow( _parent ),
	m_rememberLogin( false ),
	m_wolframeClient( 0 )
{
}

void SkeletonMainWindow::create( )
{
// setup designer UI
	initializeUi( );
		
// add connection and encryption state indicators to status bar
	addStatusBarIndicators( );

// update menus and toolbars
	updateMenusAndToolbars( );
}

void SkeletonMainWindow::initializeUi( )
{
	m_ui = new Ui::SkeletonMainWindow( );
	static_cast<Ui::SkeletonMainWindow *>( m_ui )->setupUi( this );

}
void SkeletonMainWindow::deleteUi( )
{
	delete static_cast<Ui::SkeletonMainWindow *>( m_ui );
}

void SkeletonMainWindow::addStatusBarIndicators( )
{
	m_statusBarConn = new QLabel( this );
	m_statusBarConn->setFrameStyle( QFrame::Panel | QFrame::Sunken );
	statusBar( )->addPermanentWidget( m_statusBarConn );
	m_statusBarConn->setPixmap( QPixmap( ":/images/16x16/disconnected.png" ) );
	m_statusBarConn->setToolTip( tr( "Status: offline" ) );
	m_statusBarConn->setEnabled( false );

	m_statusBarSSL = new QLabel( this );
	m_statusBarSSL->setFrameStyle( QFrame::Panel | QFrame::Sunken );
	statusBar( )->addPermanentWidget( m_statusBarSSL );
	m_statusBarSSL->setPixmap( QPixmap( ":/images/16x16/unencrypted.png" ) );
	m_statusBarSSL->setToolTip( tr( "Encryption: N/A" ) );
	m_statusBarSSL->setEnabled( false );
}

void SkeletonMainWindow::updateMenusAndToolbars( )
{
// connection status
	if( m_wolframeClient && m_wolframeClient->isConnected( ) ) {
		m_statusBarConn->setPixmap( QPixmap( ":/images/16x16/connected.png" ) );
//		m_statusBarConn->setToolTip( tr( "Status: online" ) );
		m_statusBarConn->setToolTip( tr( "Status: connected to server %1" ).arg( m_wolframeClient->serverName()) );
		m_statusBarConn->setEnabled( true );
	} else {
		m_statusBarConn->setPixmap( QPixmap( ":/images/16x16/disconnected.png" ) );
		m_statusBarConn->setToolTip( tr( "Status: offline" ) );
		m_statusBarConn->setEnabled( false );
	}
	if( m_wolframeClient && m_wolframeClient->isEncrypted( ) ) {
		m_statusBarSSL->setPixmap( QPixmap( ":/images/16x16/encrypted.png" ) );
		m_statusBarSSL->setToolTip( tr( "Encryption: %1" ).arg( m_wolframeClient->encryptionName()) );
		m_statusBarSSL->setEnabled( true );
	} else {
		m_statusBarSSL->setPixmap( QPixmap( ":/images/16x16/unencrypted.png" ) );
		m_statusBarSSL->setToolTip( tr( "Encryption: N/A" ) );
		m_statusBarSSL->setEnabled( false );
	}

	activateAction( "actionLogin", !m_wolframeClient || !m_wolframeClient->isConnected( ) );
	activateAction( "actionLogout", m_wolframeClient && m_wolframeClient->isConnected( ) );
}

void SkeletonMainWindow::error( QString error )
{
	QMessageBox::warning( this, tr( "Server error" ), error, QMessageBox::Ok );

	updateMenusAndToolbars( );
}

void SkeletonMainWindow::connected( )
{
	qDebug( ) << "Connected to server";
	
	m_wolframeClient->auth( );
}

void SkeletonMainWindow::disconnected( )
{	
	qDebug( ) << "Disconnected from server";

	statusBar( )->showMessage( tr( "Terminated" ) );

	if( m_wolframeClient ) {
		m_wolframeClient->deleteLater( );
		m_wolframeClient = 0;
	}
	
// update status of menus and toolbars
	updateMenusAndToolbars( );
}

void SkeletonMainWindow::authOk( )
{
	qDebug( ) << "Authentication succeeded";

	statusBar( )->showMessage( tr( "Ready" ) );

// update status of menus and toolbars
	updateMenusAndToolbars( );	
}

void SkeletonMainWindow::authFailed( )
{
	QMessageBox::warning( this, tr( "Authentication error" ), tr( "Authentication failed" ), QMessageBox::Ok );
}

SkeletonMainWindow::~SkeletonMainWindow( )
{
	deleteUi( );
	
	if( m_wolframeClient ) {
		qDebug( ) << "Disconnecting from server";
		m_wolframeClient->disconnect( );
	}
}

void SkeletonMainWindow::activateAction( const QString name, bool enabled )
{
	QAction *action = findChild<QAction *>( name );
	if( action ) {
		action->setEnabled( enabled );
	}
}

void SkeletonMainWindow::on_actionExit_triggered( )
{
	close( );
}

QString SkeletonMainWindow::lastUsername( ) const
{
	return m_lastUsername;
}

void SkeletonMainWindow::setLastUsername( QString &lastUsername )
{
	m_lastUsername = lastUsername;
}

QString SkeletonMainWindow::lastConnName( ) const
{
	return m_lastConnName;
}

void SkeletonMainWindow::setLastConnName( QString &lastConnName )
{
	m_lastConnName = lastConnName;
}

void SkeletonMainWindow::setRememberLogin( bool enable )
{
	m_rememberLogin = enable;
}

void SkeletonMainWindow::on_actionLogin_triggered( )
{
	login( );
}

void SkeletonMainWindow::login( )
{
	QString username;
	QString lastConn;
	
	if( m_rememberLogin ) {
		username = m_lastUsername;
		lastConn = m_lastConnName;
	}
	
	LoginDialog* loginDlg = new LoginDialog( username, lastConn,
						 m_connections );
						 
	if( loginDlg->exec( ) == QDialog::Accepted ) {
// user choose nothing, bail out
		if( !loginDlg->hasSelectedConnection( ) ) {
			QMessageBox::critical( this, tr( "Parameters error"),
				"You didn't select any connection!" );
			delete loginDlg;
			return;
		}

// optionally remember old login data
		if( m_rememberLogin ) {
			m_lastUsername = loginDlg->username( );
			m_lastConnName = loginDlg->selectedConnection( ).name;
		}
			
		ConnectionParameters selectedConnection = loginDlg->selectedConnection( );

// no SSL compiled in and the user picks a secure connection, warn him,
// don't blindly connect
		if( !WolframeClient::SSLsupported( ) && selectedConnection.SSL ) {
			QMessageBox::critical( this, tr( "Parameters error"),
				"No SSL support is compiled in, can't open a secure connection" );
			delete loginDlg;
			return;
		}

// create a Wolframe protocol client
		m_wolframeClient = new WolframeClient( selectedConnection );

// catch signals from the network layer
		connect( m_wolframeClient, SIGNAL( error( QString ) ),
			this, SLOT( error( QString ) ) );
		connect( m_wolframeClient, SIGNAL( connected( ) ),
			this, SLOT( connected( ) ) );
		connect( m_wolframeClient, SIGNAL( disconnected( ) ),
			this, SLOT( disconnected( ) ) );
		connect( m_wolframeClient, SIGNAL( authOk( ) ),
			this, SLOT( authOk( ) ) );
		connect( m_wolframeClient, SIGNAL( authFailed( ) ),
			this, SLOT( authFailed( ) ) );
		
		qDebug( ) << "Connecting to " << selectedConnection.toString( );
		
// initiate connect
		m_wolframeClient->connect( );
	}
}

void SkeletonMainWindow::on_actionLogout_triggered( )
{
	logout( );
}

void SkeletonMainWindow::logout( )
{
	m_wolframeClient->disconnect( );
}

void SkeletonMainWindow::on_actionManageServers_triggered( )
{
	ManageServersDialog* serversDlg = new ManageServersDialog( m_connections );
	serversDlg->exec( );

	delete serversDlg;
}
