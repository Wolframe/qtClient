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

#include "WImageListWidgetPlugin.hpp"
#include "WImageListWidget.hpp"

#include <QtPlugin>

WImageListWidgetPlugin::WImageListWidgetPlugin( QObject *_parent )
	: QObject( _parent ), m_initialized( false )
{
}

bool WImageListWidgetPlugin::isContainer( ) const
{
	return false;
}

bool WImageListWidgetPlugin::isInitialized( ) const
{
	return m_initialized;
}

QIcon WImageListWidgetPlugin::icon( ) const
{
	return QIcon( );
}

QString WImageListWidgetPlugin::domXml( ) const
{
	return 	"<ui language=\"c++\">\n"
	" <widget class=\"WImageListWidget\" name=\"WImageListWidget\">\n"
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

QString WImageListWidgetPlugin::group( ) const
{
	return "Wolframe Widgets";
}

QString WImageListWidgetPlugin::includeFile( ) const
{
	return "WImageListWidget.hpp";
}

QString WImageListWidgetPlugin::name( ) const
{
	return "WImageListWidget";
}

QString WImageListWidgetPlugin::toolTip( ) const
{
	return "";
}

QString WImageListWidgetPlugin::whatsThis( ) const
{
	return "";
}

QWidget *WImageListWidgetPlugin::createWidget( QWidget *_parent )
{
	// is overridden by property
	return new WImageListWidget( _parent );
}

void WImageListWidgetPlugin::initialize( QDesignerFormEditorInterface* /* core */ )
{
	if( m_initialized ) return;

	m_initialized = true;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2( wimagelistwidget, WImageListWidgetPlugin )
#endif // QT_VERSION < 0x050000
