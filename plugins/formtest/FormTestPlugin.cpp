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

#include "FormTestPlugin.hpp"

#include <QDebug>
#include <QByteArray>
#include <QVBoxLayout>
#include <QPushButton>
#include <QSpacerItem>

// FormTestWidget

FormTestWidget::FormTestWidget( FormTestPlugin *_plugin, QWidget *_parent )
	: QWidget( _parent ), m_plugin( _plugin )
{
	initialize( );
}

void FormTestWidget::initialize( )
{
	QVBoxLayout *vLayout = new QVBoxLayout( this );

	QHBoxLayout *hLayout = new QHBoxLayout( );

	QSpacerItem *hSpacer = new QSpacerItem( 20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding );
	hLayout->addItem( hSpacer );
	
	QPushButton *pressMeButton = new QPushButton( "Press me!", this );
	hLayout->addWidget( pressMeButton );
	
	QPushButton *clearButton = new QPushButton( "Clear", this );
	hLayout->addWidget( clearButton );

	vLayout->addLayout( hLayout );

	QSpacerItem *vSpacer = new QSpacerItem( 20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding );
	vLayout->addItem( vSpacer );

	m_label = new QLabel( "Form plugin test", this );
	m_label->setWordWrap( true );
	m_label->setTextFormat( Qt::RichText );
	vLayout->addWidget( m_label );

	connect( pressMeButton, SIGNAL( clicked( ) ), this, SLOT( handlePressMeButton( ) ) );
	connect( clearButton, SIGNAL( clicked( ) ), this, SLOT( handleClearButton( ) ) );
}

void FormTestWidget::handlePressMeButton( )
{
	QByteArray request;
	request.append( QString( "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>" ) );
	request.append( QString( "<!DOCTYPE \"employee\" SYSTEM \"ListEmployee.simpleform\">" ) );
	request.append( QString( "<employee/>" ) );

	m_plugin->sendRequest( winId( ), request );
}

void FormTestWidget::gotAnswer( const QByteArray& _data )
{
	QString xml( _data.data( ) );
	
	xml.replace( '&', "&amp;" ).replace( '<', "&lt;" ).replace( '>', "&gt;<br/>" );

	m_label->setText( QString( "<html><body>%2</body></html>" ).arg( xml ) );
}

void FormTestWidget::handleClearButton( )
{
	m_label->setText( "" );
}

// FormTestPlugin

FormTestPlugin::FormTestPlugin( ) : QObject( ),
	m_tagCounter( 0 )
{
}

QString FormTestPlugin::name( ) const
{
	return "test";
}

QString FormTestPlugin::windowTitle( ) const
{
	return "Test Form";
}

QWidget *FormTestPlugin::createForm( DataLoader *_dataLoader, QWidget *_parent )
{
	qDebug( ) << "PLUGIN: creating a form of type" << name( );
	
	m_dataLoader = _dataLoader;
	
	FormTestWidget *widget = new FormTestWidget( this, _parent );
	QString winId = QString::number( (int)widget->winId( ) );
	m_widgets.insert( winId, widget );
	
	return widget;
}

void FormTestPlugin::sendRequest( WId wid, const QByteArray &_request )
{
	// TODO: how to initialize this one? With what?
	QString cmd;
	QString id = QString::number( (int)wid );
	QString tag = QString( "%1:%2" ).arg( id ).arg( m_tagCounter++ );

	m_dataLoader->datarequest( cmd, tag, _request );	
}

void FormTestPlugin::gotAnswer( const QString& _tag, const QByteArray& _data )
{
	QStringList parts = _tag.split( ':' );
	QHash<QString, FormTestWidget *>::const_iterator it = m_widgets.find( parts[0] );
	if( it == m_widgets.end( ) ) {
		qDebug( ) << "Unknown tag" << _tag << ", don't know where to deliver the message";
		return;
	}
	
	FormTestWidget *widget = *it;
	if( widget ) {
		widget->gotAnswer( _data );
	}
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2( formtest, FormTestPlugin )
#endif // QT_VERSION < 0x050000
