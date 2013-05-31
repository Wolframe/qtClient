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

#include "WolfDesignerPlugin.hpp"
#include "WolfDesignerMenu.hpp"
#include "WolfDesigner.hpp"

#include <QtPlugin>

#include <QExtensionManager>
#include <QDesignerFormEditorInterface>
#include <QDesignerFormWindowManagerInterface>

#include <QLabel>
 
WolfDesignerPlugin::WolfDesignerPlugin( QObject *_parent )
	: QObject( _parent ), m_initialized( false )
{
}

bool WolfDesignerPlugin::isContainer( ) const
{
	return false;
}

bool WolfDesignerPlugin::isInitialized( ) const
{
	return m_initialized;
}

QIcon WolfDesignerPlugin::icon( ) const
{
	return QIcon( );
}

QString WolfDesignerPlugin::domXml( ) const
{
	return 	"<ui language=\"c++\">\n"
		" <widget class=\"WolfDesigner\" name=\"wolfDesigner\">\n"
		"  <property name=\"geometry\">\n"
		"   <rect>\n"
		"    <x>0</x>\n"
		"    <y>0</y>\n"
		"    <width>100</width>\n"
		"    <height>100</height>\n"
		"   </rect>\n"
		"  </property>\n"
		" </widget>\n"
		"</ui>\n";
}

QString WolfDesignerPlugin::group( ) const
{
	return "Wolframe Widgets";
}

QString WolfDesignerPlugin::includeFile( ) const
{
	return "WolfDesigner.hpp";
}
 
QString WolfDesignerPlugin::name( ) const
{
	return "WolfDesigner";
}

QString WolfDesignerPlugin::toolTip( ) const
{
	return "";
}

QString WolfDesignerPlugin::whatsThis( ) const
{
	return "";
}

QWidget *WolfDesignerPlugin::createWidget( QWidget *_parent )
{
	return new WolfDesigner( _parent );
}

void WolfDesignerPlugin::initialize( QDesignerFormEditorInterface *_formEditor )
{
	if( m_initialized ) return;
	
	QExtensionManager *extManager = _formEditor->extensionManager( );
	Q_ASSERT( extManager != 0 );

	extManager->registerExtensions( new WolfDesignerMenuFactory( extManager ),
		Q_TYPEID( QDesignerTaskMenuExtension ) );
			
	m_initialized = true;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2( wolfdesigner, WolfDesignerPlugin )
#endif // QT_VERSION < 0x050000
