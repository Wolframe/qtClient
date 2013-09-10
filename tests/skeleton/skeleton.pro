TEMPLATE = app

TARGET = skeletonTest

CONFIG += qt warn_on

DEFINES += LIBWOLFRAMECLIENT_VISIBILITY=Q_DECL_IMPORT SKELETON_VISIBILITY=Q_DECL_IMPORT

QT += core network

contains(QT_VERSION,^5\\..*) {
QT += widgets
}

QMAKE_LIBDIR = ../../libqtwolfclient ../../skeleton
unix:LIBS += -lqtwolfclient
unix:LIBS += -lskeleton
#unix:LIBS += -L../../libqtwolfclient -lqtwolfclient -L../../skeleton -lskeleton
win32:LIBS += ../../libqtwolfclient/debug/qtwolfclient0.lib ../../skeleton/debug/skeleton0.lib

INCLUDEPATH += ../../libqtwolfclient ../../skeleton

#unix:PRE_TARGETDEPS += ../../libqtwolfclient/libqtwolfclient.so ../../skeleton/libskeleton.so
win32:PRE_TARGETDEPS += ../../libqtwolfclient/debug/qtwolfclient0.lib ../../skeleton/debug/skeleton0.lib

SOURCES += \
	main.cpp

RESOURCES = \
	skeletonTest.qrc

test.depends = first

QMAKE_EXTRA_TARGETS += test

