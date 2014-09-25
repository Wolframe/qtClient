TEMPLATE = app

TARGET = example

CONFIG += qt warn_on

DEFINES += LIBWOLFRAMECLIENT_VISIBILITY=Q_DECL_IMPORT SKELETON_VISIBILITY=Q_DECL_IMPORT

INCLUDEPATH += ../skeleton ../libqtwolfclient

QT += core network

contains(QT_VERSION,^5\\..*) {
QT += widgets
}

QMAKE_RPATHDIR += $$QMAKE_LIBDIR_X11

unix:!macx:LIBS += -L../libqtwolfclient -lqtwolfclient -L../skeleton -lskeleton
win32:LIBS += ../libqtwolfclient/debug/qtwolfclient0.lib ../skeleton/debug/skeleton0.lib
macx:LIBS += ../libqtwolfclient/build/Debug/libqtwolfclient.dylib ../skeleton/build/Debug/libskeleton.dylib


SOURCES += \
	main.cpp \
	MainWindow.cpp
	
HEADERS += \
	MainWindow.hpp

FORMS += \
	MainWindow.ui

RESOURCES = \
	example.qrc

test.depends = first

QMAKE_EXTRA_TARGETS += test
