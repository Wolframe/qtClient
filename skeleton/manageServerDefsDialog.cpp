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

#include <QDialog>
#include <QMessageBox>
#include <QtGui>
#include <cassert>

#include "manageServerDefsDialog.hpp"
#include "ui_manageServerDefsDialog.h"

#include "serverDefinitionDialog.hpp"

// if set, when a definition management dialog is created
// and there are no definitions, a new definition dialog
// is automatically called
static const bool AUTO_NEW_DEF_ON_EMPTY = true;

// model
QVariant ManageServerDefsDialog::DefsListModel::data( const QModelIndex &index, int role ) const
{
	switch( role )	{
		case Qt::DisplayRole:
			return m_defsVector[ index.row() ].name;
			break;
		case Qt::FontRole:
			if ( m_defsVector[ index.row() ].name == m_defEntry )	{
				QFont boldFont;
				boldFont.setBold( true );
				return boldFont;
			}
			break;
		case Qt::BackgroundRole:
			break;
		case Qt::TextAlignmentRole:
			break;
		case Qt::CheckStateRole:
			break;
	}
	return QVariant();
}

void ManageServerDefsDialog::DefsListModel::rowChanged( const int row )
{
	if ( row >= 0 )	{
		QModelIndex r = createIndex( row, 0 );
		emit dataChanged( r, r );
	}
}

bool ManageServerDefsDialog::DefsListModel::appendServerDefinition( const ServerDefinition& def )
{
	beginInsertRows( QModelIndex(), m_defsVector.count(), m_defsVector.count() );
	m_defsVector.append( def );
	endInsertRows();
	return true;
}

bool ManageServerDefsDialog::DefsListModel::removeServerDefinition( int position )
{
	if ( position >= 0 && position < m_defsVector.count() )	{
		beginRemoveRows( QModelIndex(), position, position );
		m_defsVector.remove( position );
		endRemoveRows();
		return true;
	}
	else
		return false;
}


// the dialog
ManageServerDefsDialog::ManageServerDefsDialog( QVector<ServerDefinition>& params,
						QString& defaultServer, QWidget *parent ) :
	QDialog( parent ), ui( new Ui::ManageServerDefsDialog ),
	m_localParams( params ), m_localDefault( defaultServer ),
	m_globalParams( params ), m_globalDefault( defaultServer ),
	m_defsListModel( m_localParams, m_localDefault )
{
	ui->setupUi( this );
	ui->definitionList->setModel( & m_defsListModel );
	ui->definitionList->setSelectionMode( QAbstractItemView::SingleSelection );

	connect( ui->newBttn, SIGNAL( clicked() ), this, SLOT( newServerDefinition() ));
	connect( ui->editBttn, SIGNAL( clicked() ), this, SLOT( editServerDefinition() ));
	connect( ui->deleteBttn, SIGNAL( clicked() ), this, SLOT( delServerDefinition() ));
	connect( ui->defaultBttn, SIGNAL( clicked() ), this, SLOT( setDefaultServer() ));
	connect( ui->definitionList->selectionModel(),
		 SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
		 this, SLOT( updateUIstate() ));

	updateUIstate();
}

ManageServerDefsDialog::~ManageServerDefsDialog()
{
	delete ui;
}


// override exec()
int ManageServerDefsDialog::exec()
{
	if ( AUTO_NEW_DEF_ON_EMPTY && m_localParams.empty() )
		newServerDefinition();
	return QDialog::exec();
}

void ManageServerDefsDialog::done( int retCode )
{
	if ( retCode == QDialog::Accepted )	{
		m_globalParams.clear();
		m_globalParams = m_localParams;
		m_globalDefault = m_localDefault;
		QDialog::done( retCode );
	}
	else
		QDialog::done( retCode );
}

void ManageServerDefsDialog::newServerDefinition()
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
			if ( ! duplicate )
				m_defsListModel.appendServerDefinition( conn );
		}
	} while( duplicate );

	delete newConn;
}

void ManageServerDefsDialog::editServerDefinition()
{
	QList< QModelIndex > items = ui->definitionList->selectionModel()->selectedIndexes();
	assert( items.size() == 1 );

	int pos = items.begin()->row();
	assert( pos >= 0 && pos < m_localParams.size() );

	ServerDefinition conn = m_localParams[ pos ];
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
				m_defsListModel.rowChanged( pos );
			}
		}
	} while( duplicate );

	delete editServer;
}

void ManageServerDefsDialog::delServerDefinition()
{
	QList< QModelIndex > items = ui->definitionList->selectionModel()->selectedIndexes();
	assert( items.size() == 1 );

	int pos = items.begin()->row();
	assert( pos >= 0 && pos < m_localParams.size() );

	QString msg = QString( "Delete the definition for '%1' ?" ).arg( m_localParams[ pos ].name );
	QMessageBox::StandardButton ret = QMessageBox::question( this, tr( "Delete definition" ),
								 msg, QMessageBox::Ok | QMessageBox::Cancel );
	if ( ret == QMessageBox::Ok )
		m_defsListModel.removeServerDefinition( pos );
}

void ManageServerDefsDialog::setDefaultServer()
{
	QList< QModelIndex > items = ui->definitionList->selectionModel()->selectedIndexes();
	assert( items.size() == 1 );

	// get current default
	int oldPos = -1;
	for ( int i = 0; i < m_localParams.size(); i++ )	{
		if ( m_localParams[ i ].name.compare( m_localDefault, Qt::CaseInsensitive ) == 0 )	{
			oldPos = i;
			break;
		}
	}

	m_localDefault = m_localParams[ items.begin()->row() ].name;
	ui->defaultBttn->setEnabled( false );
	m_defsListModel.rowChanged( oldPos );
	m_defsListModel.rowChanged( items.begin()->row() );
}

void ManageServerDefsDialog::updateUIstate()
{
	// Update the states of the buttons
	QList< QModelIndex > items = ui->definitionList->selectionModel()->selectedIndexes();
	if ( items.empty() )	{
		ui->newBttn->setEnabled( true );
		ui->editBttn->setEnabled( false );
		ui->deleteBttn->setEnabled( false );
		ui->defaultBttn->setEnabled( false );
	}
	else	{
		ui->newBttn->setEnabled( true );
		if ( items.count() == 1 )
			ui->editBttn->setEnabled( true );
		else
			ui->editBttn->setEnabled( false );
		ui->deleteBttn->setEnabled( true );
		if ( items.count() != 1 ||
				m_localParams[ items.begin()->row() ].name == m_localDefault )
			ui->defaultBttn->setEnabled( false );
		else
			ui->defaultBttn->setEnabled( true );
	}

	// Update the brief of the selected definition
	if ( items.count() == 1 )	{
		QString name = m_localParams[ items.begin()->row() ].name;
		for ( QVector< ServerDefinition >::const_iterator it = m_localParams.begin();
									it != m_localParams.end(); it++ )	{
			if ( it->name.compare( name, Qt::CaseInsensitive ) == 0 )	{
				ui->showParamLbl->setText( it->toString() );
				break;
			}
		}
	}
	else	{
		if ( m_localParams.count())
			ui->showParamLbl->setText( tr( "No definition selected." ) );
		else
			ui->showParamLbl->setText( tr( "No servers defined." ) );
	}
}
