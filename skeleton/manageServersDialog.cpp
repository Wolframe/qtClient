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

#include <QDialog>
#include <QMessageBox>
#include <QtGui>
#include <cassert>

#include "manageServersDialog.hpp"
#include "ui_manageServersDialog.h"

#include "serverDefinitionDialog.hpp"

ManageServersDialog::ManageServersDialog( QVector<ServerDefinition>& params,
					  QWidget *parent ) :
	QDialog( parent ), ui( new Ui::ManageServersDialog ),
	m_globalParams( params )
{
	m_localParams = m_globalParams;
	ui->setupUi( this );
	ui->connectionList->sortItems();
	ui->connectionList->setSelectionMode( QAbstractItemView::SingleSelection );
	for ( QVector<ServerDefinition>::const_iterator it = m_localParams.begin();
							it != m_localParams.end(); it++ )
		ui->connectionList->addItem( it->name );
	connect( ui->newServerBttn, SIGNAL( clicked() ), this, SLOT( newServerDefinition() ));
	connect( ui->editServerBttn, SIGNAL( clicked() ), this, SLOT( editServerDefinition() ));
	connect( ui->delServerBttn, SIGNAL( clicked() ), this, SLOT( delServerDefinition() ));
	connect( ui->connectionList, SIGNAL( itemSelectionChanged() ), this, SLOT( updateUIstate() ));

	updateUIstate();
}

ManageServersDialog::~ManageServersDialog()
{
	delete ui;
}

void ManageServersDialog::done( int retCode )
{
	if ( retCode == QDialog::Accepted )	{
		m_globalParams.clear();
		m_globalParams = m_localParams;
		QDialog::done( retCode );
	}
	else
		QDialog::done( retCode );
}

void ManageServersDialog::newServerDefinition()
{
	ServerDefinition conn;
	ServerDefinitionDialog* newConn = new ServerDefinitionDialog( conn, this );
	newConn->windowTitle() = tr( "New connection parameters" );

	bool duplicate;
	do	{
		duplicate = false;
		if ( newConn->exec() )	{
			for ( int i = 0; i < m_localParams.size(); i++ )	{
				if ( m_localParams[ i ].name.compare( conn.name, Qt::CaseInsensitive ) == 0 )	{
					duplicate = true;
					QString msg = QString( "A definition named '%1' already exists" ).arg( m_localParams[ i ].name );
					QMessageBox::critical( this, tr( "New definition" ), msg );
					break;
				}
			}
			if ( ! duplicate )	{
				m_localParams.append( conn );
				ui->connectionList->clear();
				for ( int i = 0; i < m_localParams.size(); i++ )
					ui->connectionList->addItem( m_localParams[ i ].name );
			}
		}
	} while( duplicate );

	delete newConn;
}

void ManageServersDialog::editServerDefinition()
{
	QList< QListWidgetItem* > items = ui->connectionList->selectedItems();
	assert( items.size() == 1 );

	int	pos;
	ServerDefinition conn;
	QString name = items.first()->data( 0 ).toString();
	for ( pos = 0; pos < m_localParams.size(); pos++ )	{
		if ( m_localParams[ pos ].name.compare( name, Qt::CaseInsensitive ) == 0 )	{
			conn = m_localParams[ pos ];
			break;
		}
	}
	assert( pos >= 0 && pos < m_localParams.size() );

	ServerDefinitionDialog* editServer = new ServerDefinitionDialog( conn, this );
	editServer->windowTitle() = tr( "Edit connection parameters" );

	bool duplicate;
	do	{
		duplicate = false;
		if ( editServer->exec() )	{
			for ( int i = 0; i < m_localParams.size(); i++ )	{
				if ( m_localParams[ i ].name.compare( conn.name, Qt::CaseInsensitive ) == 0 && i != pos )	{
					duplicate = true;
					QString msg = QString( "A definition named '%1' already exists" ).arg( m_localParams[ i ].name );
					QMessageBox::critical( this, tr( "Edit definition" ), msg );
					break;
				}
			}
			if ( ! duplicate )	{
				m_localParams[ pos ] = conn;
				ui->connectionList->clear();
				for ( int i = 0; i < m_localParams.size(); i++ )
					ui->connectionList->addItem( m_localParams[ i ].name );
			}
		}
	} while( duplicate );

	delete editServer;
}

void ManageServersDialog::delServerDefinition()
{
	QList< QListWidgetItem* > items = ui->connectionList->selectedItems();
	assert( items.size() == 1 );

	QString conn = items.first()->data( 0 ).toString();
	for ( int i = 0; i < m_localParams.size(); i++ )	{
		if ( m_localParams[ i ].name.compare( conn, Qt::CaseInsensitive ) == 0 )	{
			QString msg = QString( "Delete the definition for '%1' ?" ).arg( m_localParams[ i ].name );
			QMessageBox::StandardButton ret = QMessageBox::question( this, tr( "Delete definition" ),
										 msg, QMessageBox::Ok | QMessageBox::Cancel );
			if ( ret == QMessageBox::Ok )	{
				m_localParams.remove( i );
				ui->connectionList->clear();
				for ( int i = 0; i < m_localParams.size(); i++ )
					ui->connectionList->addItem( m_localParams[ i ].name );
			}
			break;
		}
	}
}

void ManageServersDialog::updateUIstate()
{
	// Update the states of the buttons
	QList< QListWidgetItem* > items = ui->connectionList->selectedItems();
	if ( items.empty() )	{
		ui->newServerBttn->setEnabled( true );
		ui->editServerBttn->setEnabled( false );
		ui->delServerBttn->setEnabled( false );
	}
	else	{
		ui->newServerBttn->setEnabled( true );
		ui->editServerBttn->setEnabled( true );
		ui->delServerBttn->setEnabled( true );
	}

	// Update the brief of the selected definition
	if ( ui->connectionList->count() && ui->connectionList->currentItem() )	{
		QString name = ui->connectionList->currentItem()->text();
		for ( QVector< ServerDefinition >::const_iterator it = m_localParams.begin();
									it != m_localParams.end(); it++ )	{
			if ( it->name.compare( name, Qt::CaseInsensitive ) == 0 )	{
				ui->parametersLbl->setText( it->toString() );
				break;
			}
		}
	}
	else
		ui->parametersLbl->setText( tr( "No definition selected." ) );
}
