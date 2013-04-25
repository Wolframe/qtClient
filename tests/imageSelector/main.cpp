
#include <QtGui>

#include "pluginWindow.hpp"

int main( int argv, char *args[] )
{
	QApplication app( argv, args );
	PluginWindow window;
	window.show();
	return app.exec();
}
