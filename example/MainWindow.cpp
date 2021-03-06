/************************************************************************

 Copyright (C) 2011 - 2014 Project Wolframe.
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

#include "ui_MainWindow.h"

#include <QMessageBox>

MainWindow::MainWindow( QWidget *_parent ) : SkeletonMainWindow( _parent )
{
// a demo connection, goes away later
	ServerDefinition serverDef;

	serverDef.name = "example";
	serverDef.host = "localhost";
	serverDef.port = 7661;

	m_serverDefs.append( serverDef );

	setRememberLogin( true );
}

MainWindow::~MainWindow( )
{
}

void MainWindow::initializeUi( )
{
	m_ui = new Ui::MainWindow( );
	static_cast<Ui::MainWindow *>( m_ui )->setupUi( this );
}

void MainWindow::deleteUi( )
{
	delete static_cast<Ui::MainWindow *>( m_ui );
}

void MainWindow::authOk( )
{
	SkeletonMainWindow::authOk( );

	QMessageBox::warning( this, tr( "Authentication" ), tr( "authentication succeeded.." ), QMessageBox::Ok );
}

void MainWindow::login( )
{
	QMessageBox::warning( this, tr( "Authentication" ), tr( "logging in.." ), QMessageBox::Ok );

	SkeletonMainWindow::login( );
}

void MainWindow::on_actionDoSomething_triggered( )
{
	QMessageBox::warning( this, tr( "Do something" ), tr( "doing nice things here.." ), QMessageBox::Ok );
}
