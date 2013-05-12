#include <QtCore>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif
#include <QtGui>

#include "pluginWindow.hpp"

int main( int argv, char *args[] )
{
	QApplication app( argv, args );
	PluginWindow window;
	window.show();
	return app.exec();
}
