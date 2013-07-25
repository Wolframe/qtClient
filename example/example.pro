TEMPLATE = app

TARGET = example

CONFIG += qt warn_on

DEFINES += LIBWOLFRAMECLIENT_VISIBILITY=Q_DECL_IMPORT SKELETON_VISIBILITY=Q_DECL_IMPORT

INCLUDEPATH += ../skeleton ../libqtwolframeclient

QT += core network

contains(QT_VERSION,^5\\..*) {
QT += widgets
}

unix:LIBS += -L../libqtwolframeclient -lqtwolframeclient -L../skeleton -lskeleton
win32:LIBS += ../libqtwolframeclient/debug/qtwolframeclient0.lib ../skeleton/debug/skeleton0.lib

unix;PRE_TARGETDEPS += ../libqtwolframeclient/libqtwolframeclient.so ../skeleton/libskeleton.so
win32:PRE_TARGETDEPS += ../libqtwolframeclient/debug/qtwolframeclient0.lib ../skeleton/debug/skeleton0.lib

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
