#ifndef _APPLICATION_HPP_INCLUDED
#define _APPLICATION_HPP_INCLUDED

#include <QApplication>
#include <QHash>
#include <QStringList>

class QDir;
class QTranslator;

typedef QHash< QString, QTranslator* > Translators;

class Application : public QApplication
{
	Q_OBJECT

public:
	explicit Application( int& argc, char* argv[] );
	~Application();

	static void loadTranslations( const QString& dir );
	static void loadTranslations( const QDir& dir );
	static const QStringList availableLanguages();

public slots:
	static void setLanguage( const QString& locale );

private:
	static QTranslator*	current;
	static Translators	translators;
};

#endif // _APPLICATION_HPP_INCLUDED
