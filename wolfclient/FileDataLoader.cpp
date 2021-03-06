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

#include "FileDataLoader.hpp"

#include <QDir>
#include <QFile>
#include <QDebug>

FileDataLoader::FileDataLoader( QString dir ) : m_dir( dir )
{
}

void FileDataLoader::datarequest( const QString& cmd, const QString& tag, const QByteArray& content)
{
	qDebug( ) << "datarequest, cmd" << cmd << ", tag" << tag << "\n" << content;
}

void FileDataLoader::gotAnswer( bool /*success*/, const QString& /*tag*/, const QByteArray& /*content*/)
{
	// skip, answer signal is emitted above, we don't wait for network
	// signals here
}

/*
void FileDataLoader::request( QString windowName, QString formName, QString widgetName, QByteArray xml, QHash<QString, QString> *props )
{
	qDebug( ) << "Request" << windowName << formName << widgetName << *props;

// nothing to do
	if( !props->contains( "action" ) ) {
		return;
	}

// handle CRUD actions, they are file system dataloader specific
	QString action = props->value( "action" );
	if( action == "create" ) {
		handleCreate( formName, xml, props );
	} else if( action == "read" ) {
		if( widgetName.isEmpty( ) ) {
			handleRead( formName, props );
		} else {
			handleDomainDataLoad( formName, widgetName, props );
		}
	} else if( action == "update" ) {
		handleUpdate( formName, xml, props );
	} else if( action == "delete" ) {
		handleDelete( formName, props );
	}
}

void FileDataLoader::handleCreate( QString name, QByteArray xml, QHash<QString, QString> *props )
{
	QFile file( m_dir + "/" + name + ".xml" );
	file.open( QFile::WriteOnly );
	file.write( xml );
	file.close( );
	emit answer( (*props)[ "widgetid"], QByteArray( ) );
}

void FileDataLoader::handleRead( QString name, QHash<QString, QString> *props )
{
// read directly here and stuff data into the signal
	QFile file( m_dir + "/" + name + ".xml" );
	file.open( QFile::ReadOnly );
	QByteArray xml = file.readAll( );
	file.close( );
	emit answer( (*props)[ "widgetid"], xml );
}

void FileDataLoader::handleUpdate( QString name, QByteArray xml, QHash<QString, QString> *props )
{
	QFile file( m_dir + "/" + name + ".xml" );
	file.open( QFile::WriteOnly );
	file.write( xml );
	file.close( );
	emit answer( (*props)[ "widgetid"], QByteArray( ) );
}

void FileDataLoader::handleDelete( QString name, QHash<QString, QString> *props )
{
	QFile file( m_dir + "/" + name + ".xml" );
	file.remove( );
	emit answer( (*props)[ "widgetid"], QByteArray( ) );
}

void FileDataLoader::handleDomainDataLoad( QString formName, QString widgetName, QHash<QString, QString> *props )
{
	QFile file( m_dir + "/domain_" + formName + "_" + widgetName + ".xml" );
	file.open( QFile::ReadOnly );
	QByteArray xml = file.readAll( );
	file.close( );
	emit answer( (*props)[ "widgetid"], xml );
}

NetworkDataLoader::NetworkDataLoader( WolframeClient *_wolframeClient, bool _debug )
	: m_wolframeClient( _wolframeClient ),
	  m_map( new QHash<QByteArray, QPair<QString, QString> >( ) ),
	  m_debug( _debug )
{
	connect( m_wolframeClient, SIGNAL( answerReceived( bool,const QString&,const QByteArray&) ),
		this, SLOT( gotAnswer( bool,const QString&,const QByteArray&) ) );
}

void NetworkDataLoader::datarequest( const QString& tag, const QByteArray& content)
{
	m_wolframeClient->request( tag, content);
}

void NetworkDataLoader::gotAnswer( bool success, const QString& tag, const QByteArray& content )
{
	if( !success ) {
		qCritical( ) << "ERROR: " << tag << content;
		emit error( tag, content);
		return;
	}
	emit answer( tag, content);
}

*/
