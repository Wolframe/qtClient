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

#ifndef _SKELETON_MAIN_WINDOW_HPP_INCLUDED
#define _SKELETON_MAIN_WINDOW_HPP_INCLUDED

#include <QMainWindow>
#include <QVector>
#include <QString>

#include "serverDefinition.hpp"

class WolframeClient;
class QLabel;

class SKELETON_VISIBILITY SkeletonMainWindow : public QMainWindow
{
	Q_OBJECT

	public:
		SkeletonMainWindow( QWidget *_parent = 0 );
		virtual ~SkeletonMainWindow( );
		virtual void create( );

	protected:
		void addStatusBarIndicators( );
		void activateAction( const QString name, bool enabled );

		virtual void initializeUi( );
		virtual void deleteUi( );
		virtual void updateMenusAndToolbars( );

		QString lastUsername( ) const;
		void setLastUsername( QString &username );
		QString lastConnName( ) const;
		void setLastConnName( QString &connName );
		void setRememberLogin( bool enable );

	private:
		bool m_rememberLogin;
		QLabel *m_statusBarConn;
		QLabel *m_statusBarSSL;
		QString	m_lastUsername;
		QString	m_lastConnName;

	protected:
		void *m_ui;
		WolframeClient *m_wolframeClient;
		QVector<ServerDefinition> m_serverDefs;
		QString	m_defaultServer;

	protected slots:
// slots for the wolframe client
		virtual void error( QString error );
		virtual void connected( );
		virtual void disconnected( );
		virtual void authOk( );
		virtual void authFailed( );
		virtual void login( );
		virtual void logout( );

	private slots:
// auto-wired slots for the menu
		void on_actionExit_triggered( );
		void on_actionLogin_triggered( );
		void on_actionLogout_triggered( );
		void on_actionManageServers_triggered( );
};

#endif // _SKELETON_MAIN_WINDOW_HPP_INCLUDED
