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

#include "FileFormLoader.hpp"
#include "FormCall.hpp"
#include "global.hpp"

#include <QDir>
#include <QFile>
#include <QResource>

#include <QDebug>

#include <QPluginLoader>
#include <QApplication>
#include "FormPluginInterface.hpp"

FileFormLoader::FileFormLoader( QString formDir, QString localeDir, QString resourcesDir, QString menusDir )
	: m_formDir( formDir ), m_localeDir( localeDir ), m_resourcesDir( resourcesDir ),
	m_menusDir( menusDir )
{
	initialize( );
}

void FileFormLoader::initialize( )
{
	// register all resources in the resources form dir to the application
	QDir resourcesDir( m_resourcesDir );
	QStringList nameFilters;
	nameFilters << "*.rcc";
	resourcesDir.setNameFilters( nameFilters );
	QFileInfoList resources = resourcesDir.entryInfoList( QDir::Files | QDir::NoDotAndDotDot, QDir::Name );
	foreach( QFileInfo resource, resources ) {
		QString filename = resource.filePath( );
		qDebug( ) << "Loading resource file" << filename;
		if( !QResource::registerResource( filename ) ) {
			qWarning( ) << "Unable to open resource file" << filename;
		}
	}
}

void FileFormLoader::initiateMenuListLoad( )
{
	QStringList menus;

	QDir menusDir( m_menusDir );
	QStringList filters;
	filters << "*.ui";
	menusDir.setNameFilters( filters );
	menus = menusDir.entryList( QDir::Files | QDir::NoDotAndDotDot, QDir::Name )
		.replaceInStrings( ".ui", "" );
	
	emit menuListLoaded( menus );
}

void FileFormLoader::initiateMenuLoad( QString &name )
{
// read directly here and stuff data into the signal
	QByteArray menu = readFile( m_menusDir + "/" + FormCall::name( name) + ".ui" );

	emit menuLoaded( name, menu );
}

void FileFormLoader::initiateListLoad( )
{
	QStringList forms;
	
// fetch all ui files from the given directory
	QDir formsDir( m_formDir );
	QStringList filters;
	filters << "*.ui";
	formsDir.setNameFilters( filters );
	forms = formsDir.entryList( QDir::Files | QDir::NoDotAndDotDot, QDir::Name )
		.replaceInStrings( ".ui", "" );

// load form plugin code
	QDir pluginDir( m_formDir );
	foreach( QString filename, pluginDir.entryList( QDir::Files ) ) {
		if( !QLibrary::isLibrary( filename ) ) continue;
		QPluginLoader loader( pluginDir.absoluteFilePath( filename ) );
		QObject *object = loader.instance( );
		if( object ) {
			if( qobject_cast<FormPluginInterface *>( object ) ) {
				FormPluginInterface *plugin = qobject_cast<FormPluginInterface *>( object );
				QString name = plugin->name( );
				qDebug( ) << "PLUGIN: Loaded form plugin" << name;
				forms << name;
			} else {
				qDebug( ) << "PLUGIN:" << filename << "is not a form plugin, ignoring it";
			}
		} else {
			qWarning( ) << "PLUGIN: Loading" << filename << "failed:", loader.errorString( );
		}
	}

	emit formListLoaded( forms );
}

QByteArray FileFormLoader::readFile( QString name )
{
	qDebug( ) << "Reading file " << name;
	QFile file( name );
	file.open( QFile::ReadOnly );
	QByteArray data = file.readAll( );
	file.close( );
	return data;
}

void FileFormLoader::initiateFormLocalizationLoad( QString &name, QLocale locale )
{
	QByteArray localization = readFile( m_localeDir + "/" + FormCall::name(name) + "." + locale.name( ) + ".qm" );
	emit formLocalizationLoaded( name, localization );
}

void FileFormLoader::initiateFormLoad( QString &name )
{
// read directly here and stuff data into the signal
	QByteArray form = readFile( m_formDir + "/" + FormCall::name( name) + ".ui" );

	emit formLoaded( name, form );
}

void FileFormLoader::initiateGetLanguageCodes( )
{
	QStringList languageCodes;
	languageCodes.push_back( DEFAULT_LOCALE ); // default locale, always around

// read list of supported languages for all forms based on their qm files available
	QDir translationDir( m_localeDir );
	translationDir.setFilter( QDir::Files | QDir::NoDotAndDotDot );
	translationDir.setSorting( QDir::Name );
	QStringList filters;
	filters << "*.qm";
	translationDir.setNameFilters( filters );
	QStringList localeFiles = translationDir.entryList( );
	QMutableStringListIterator it( localeFiles );
	while( it.hasNext( ) ) {
		it.next( );
		QStringList parts = it.value( ).split( "." );
		languageCodes.push_back( parts[1] );
	}

	languageCodes.removeDuplicates( );

	emit languageCodesLoaded( languageCodes );
}

void FileFormLoader::initiateFormSave( QString /*name*/, QByteArray /*form*/ )
{
}

void FileFormLoader::initiateFormLocalizationSave( QString /*name*/, QLocale /*locale*/, QByteArray /*localizationSrc*/, QByteArray /*localizationBin*/ )
{
}

void FileFormLoader::initiateFormDelete( QString /*name*/ )
{
}

void FileFormLoader::initiateFormLocalizationDelete( QString /*name*/, QLocale /*locale*/ )
{
}

