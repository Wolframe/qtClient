#include "application.hpp"
#include "mainWindow.hpp"

int main( int argc, char* argv[] )
{
	Application app( argc, argv );
	Q_INIT_RESOURCE( translation );
	app.loadTranslations( ":/langs" );
	MainWindow w;
	w.show();
	app.connect( &app, SIGNAL( lastWindowClosed() ), &app, SLOT( quit() ) );
	return app.exec();
}
