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

unix:LIBS += -L../libqtwolframeclient -lqtwolframeclient
win32:LIBS += ../libqtwolframeclient/debug/qtwolframeclient0.lib

INCLUDEPATH += ../libqtwolframeclient

unix:PRE_TARGETDEPS += ../libqtwolframeclient/libqtwolframeclient.so
win32:PRE_TARGETDEPS += ../libqtwolframeclient/debug/qtwolframeclient0.lib

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
