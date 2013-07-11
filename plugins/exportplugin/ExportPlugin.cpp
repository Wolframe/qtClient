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

#include "ExportPlugin.hpp"

#include <QDebug>
#include <QByteArray>
#include <QVBoxLayout>
#include <QPushButton>
#include <QXmlStreamWriter>
#include <QMessageBox>
#include <QApplication>
#include <QFile>
#include <QFileInfo>

// ExportWidget

ExportWidget::ExportWidget( ExportPlugin *_plugin, const FormCall &_formCall, QWidget *_parent )
	: QWidget( _parent ), m_formCall( _formCall ), m_plugin( _plugin )
{
	initialize( );
	startExportData( );
}

void ExportWidget::initialize( )
{
	QVBoxLayout *vLayout = new QVBoxLayout( this );

	QHBoxLayout *hLayout = new QHBoxLayout( );

	QPushButton *closeButton = new QPushButton( this );
        closeButton->setText( QApplication::translate( "exportplugin", "Close", 0 ) );
	closeButton->setProperty( "form", QVariant( QString( "_CLOSE_" ) ) );
	hLayout->addWidget( closeButton );

	vLayout->addLayout( hLayout );

	m_label = new QLabel( "Exporting...", this );
	vLayout->addWidget( m_label );
}

QString ExportWidget::getFormParam( const QString &key ) const
{
	QList<FormCall::Parameter> parms = m_formCall.parameter( );
	foreach( FormCall::Parameter parm, parms ) {
		if( parm.first == key ) {
			return parm.second.toString( );
		}
	}
	return QString( "" );
}

void ExportWidget::startExportData( )
{
	QByteArray request;
	QXmlStreamWriter xml( &request );

	xml.writeStartDocument( );
	xml.writeDTD( QString( "<!DOCTYPE %1 SYSTEM '%2'>" ).arg( "export" ).arg( "ExportRequest.simpleform" ) );
	xml.writeStartElement( "", "export" );
	xml.writeEndElement( );
	xml.writeEndDocument( );

	qDebug( ) << "self-made XML request: " << request;

	m_plugin->sendRequest( winId( ), request );
}

void ExportWidget::gotAnswer( const QByteArray& _data )
{
	QString fileName = getFormParam( "filename" );
	QFileInfo finfo( fileName );
	QFile f( fileName );

	m_label->setText( QString( "Exporting to '%1'..." ).arg( fileName ) );

	f.open( QFile::WriteOnly );
	f.write( _data.constData( ), _data.length( ) );
	f.close( );

	m_label->setText( QString( "Exported to '%1'..." ).arg( fileName ) );
}

void ExportWidget::gotError( const QByteArray& _error )
{
	QMessageBox::critical( this, tr( "Form error" ), QString( _error.data( ) ), QMessageBox::Ok );
}

// ExportPlugin

ExportPlugin::ExportPlugin( ) : QObject( ),
	m_tagCounter( 0 ), m_debug( false )
{
}

QString ExportPlugin::name( ) const
{
	return "exportplugin";
}

QString ExportPlugin::windowTitle( ) const
{
	return "Export Data";
}

void ExportPlugin::setDebug( bool _debug )
{
	m_debug = _debug;
}

QWidget *ExportPlugin::createForm( const FormCall &_formCall, DataLoader *_dataLoader, bool _debug, QHash<QString,QVariant>* _globals, QWidget *_parent )
{
	m_debug = _debug;
	m_globals = _globals;
	
	qDebug( ) << "PLUGIN: creating a form of type" << name( );
	
	m_dataLoader = _dataLoader;
	
	ExportWidget *widget = new ExportWidget( this, _formCall, _parent );
	QString winId = QString::number( (int)widget->winId( ) );
	m_widgets.insert( winId, widget );
	
	return widget;
}

void ExportPlugin::sendRequest( WId wid, const QByteArray &_request )
{
	// TODO: how to initialize this one? With what?
	QString cmd;
	QString id = QString::number( (int)wid );
	QString tag = QString( "%1:%2" ).arg( id ).arg( m_tagCounter++ );

	m_dataLoader->datarequest( cmd, tag, _request );	
}

void ExportPlugin::gotAnswer( const QString& _tag, const QByteArray& _data )
{
	QStringList parts = _tag.split( ':' );
	QHash<QString, ExportWidget *>::const_iterator it = m_widgets.find( parts[0] );
	if( it == m_widgets.end( ) ) {
		qDebug( ) << "Unknown tag" << _tag << ", don't know where to deliver the message";
		return;
	}
	
	ExportWidget *widget = *it;
	if( widget ) {
		widget->gotAnswer( _data );
	}
}

void ExportPlugin::gotError( const QString& _tag, const QByteArray& _error )
{
	QStringList parts = _tag.split( ':' );
	QHash<QString, ExportWidget *>::const_iterator it = m_widgets.find( parts[0] );
	if( it == m_widgets.end( ) ) {
		qDebug( ) << "Unknown tag" << _tag << ", don't know where to deliver the error";
		return;
	}
	
	ExportWidget *widget = *it;
	if( widget ) {
		widget->gotError( _error );
	}
}

#if QT_VERSION < 0x050000
#ifdef _WIN32
#ifdef QT_NO_DEBUG
Q_EXPORT_PLUGIN2( exportplugin, ExportPlugin )
#else
Q_EXPORT_PLUGIN2( exportplugind, ExportPlugin )
#endif
#else
Q_EXPORT_PLUGIN2( exportplugin, ExportPlugin )
#endif
#endif // QT_VERSION < 0x050000
