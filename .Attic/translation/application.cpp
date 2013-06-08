#include <QDir>
#include <QFileInfo>
#include <QTranslator>
#include "application.hpp"

#include <QDebug>

QTranslator* Application::current = 0;
Translators Application::translators;

Application::Application( int& argc, char* argv[] )
	: QApplication( argc, argv )
{
}

Application::~Application()
{
}

void Application::loadTranslations( const QString& dir )
{
	loadTranslations( QDir( dir ) );
}

void Application::loadTranslations( const QDir& dir )
{
	// <language>_<country>.qm
	QString filter = "*.qm";
	QDir::Filters filters = QDir::Files | QDir::Readable;
	QDir::SortFlags sort = QDir::Name;
	qDebug() << "Translations directory:"<< dir.path();
	QFileInfoList entries = dir.entryInfoList( QStringList() << filter, filters, sort );
	foreach ( QFileInfo file, entries )
	{
		// pick country and language out of the file name
		QStringList parts = file.baseName().split( "_" );
		QString language;
		QString country;
		if ( parts.count() >= 2 )	{
			language = parts.at( parts.count() - 2 ).toLower();
			country  = parts.at( parts.count() - 1 ).toUpper();
			qDebug() << "File:" << file.fileName() << "language:" << language << "country:" << country;
		}
		else if ( parts.size() == 1 )	{
			language = parts.at( parts.count() - 1 ).toLower();
			qDebug() << "File:" << file.fileName() << "language:" << language;
		}

		// construct and load translator
		QTranslator* translator = new QTranslator( instance() );
		if ( translator->load( file.absoluteFilePath() ) )	{
			QString locale;
			if ( country.isEmpty() )	{
				locale = language;
			}
			else	{
				locale = language + "_" + country;
			}
			qDebug() << "Loaded" << file.absoluteFilePath() << "for locale" << locale;
			translators.insert( locale, translator );
		}
	}
}

const QStringList Application::availableLanguages()
{
	// the content won't get copied thanks to implicit sharing and constness
	return QStringList( translators.keys() );
}

void Application::setLanguage( const QString& locale )
{
	// remove previous
	if ( current )	{
		removeTranslator( current );
	}

	// install new
	current = translators.value( locale, 0 );
	if ( current )	{
		installTranslator( current );
	}
}
