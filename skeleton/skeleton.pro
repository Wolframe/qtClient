TEMPLATE = lib

TARGET = skeleton

isEmpty(LIBDIR) {
  LIBDIR = $${PREFIX}/lib
}

target.path = $$LIBDIR
INSTALLS += target

VERSION = 0.0.1

CONFIG += qt warn_on

DEFINES += LIBWOLFRAMECLIENT_VISIBILITY=Q_DECL_IMPORT SKELETON_VISIBILITY=Q_DECL_EXPORT

QT += core network

contains(QT_VERSION,^5\\..*) {
QT += widgets
}

QMAKE_LIBDIR = ../libqtwolfclient
#unix:LIBS += -L../libqtwolfclient -lqtwolfclient
unix:LIBS += -lqtwolfclient
win32:LIBS += ../libqtwolfclient/debug/qtwolfclient0.lib

INCLUDEPATH += ../libqtwolfclient

QMAKE_RPATHDIR += $$QMAKE_LIBDIR_X11

#unix:PRE_TARGETDEPS += ../libqtwolfclient/libqtwolfclient.so
win32:PRE_TARGETDEPS += ../libqtwolfclient/debug/qtwolfclient0.lib

SOURCES += \
	SkeletonMainWindow.cpp \
	loginDialog.cpp \
	serverDefinitionDialog.cpp \
	manageServerDefsDialog.cpp

HEADERS += \
	SkeletonMainWindow.hpp \
	loginDialog.hpp \
	serverDefinitionDialog.hpp \
	manageServerDefsDialog.hpp

FORMS += \
	SkeletonMainWindow.ui \
	loginDialog.ui \
	serverDefinitionDialog.ui \
	manageServerDefsDialog.ui

RESOURCES = \
	skeleton.qrc

test.depends = first

QMAKE_EXTRA_TARGETS += test
