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
#include <QLabel>
#include <QByteArray>
#include <QVBoxLayout>

QString FormTestPlugin::name( ) const
{
	return "test";
}

QString FormTestPlugin::windowTitle( ) const
{
	return "Test Form";
}

QWidget *FormTestPlugin::initialize( DataLoader *_dataLoader, QWidget *_parent )
{
	qDebug( ) << "PLUGIN: initializing plugin" << name( );
	
	m_dataLoader = _dataLoader;
	
	m_widget = new QWidget( _parent );
	
	QVBoxLayout *layout = new QVBoxLayout( m_widget );
	m_widget->setLayout( layout );
	
	QLabel *label = new QLabel( "Form plugin test" );
	layout->addWidget( label );

	m_pushButton = new QPushButton( "Press me!", m_widget );
	layout->addWidget( m_pushButton );

	connect( m_pushButton, SIGNAL( clicked( ) ), this, SLOT( handleButtonPress( ) ) );
	
	return m_widget;
}

void FormTestPlugin::handleButtonPress( )
{
	QString cmd;
	QString tag = "test";
	QByteArray content;

	content.append( QString( "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>" ) );
	content.append( QString( "<!DOCTYPE \"employee\" SYSTEM \"ListEmployee.simpleform\">" ) );
	content.append( QString( "<employee/>" ) );

	m_dataLoader->datarequest( cmd, tag, content );
}

void FormTestPlugin::gotAnswer( const QString& _tag, const QByteArray& _data )
{
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2( formtest, FormTestPlugin )
#endif // QT_VERSION < 0x050000
