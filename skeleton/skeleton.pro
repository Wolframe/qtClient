TEMPLATE = lib

TARGET = skeleton

VERSION = 0.0.1

CONFIG += qt debug static_and_shared

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
	manageServersDialog.cpp \
	serverDefinitionDialog.cpp
	
HEADERS += \
	SkeletonMainWindow.hpp \
	loginDialog.hpp \
	manageServersDialog.hpp \
	serverDefinitionDialog.hpp
	
FORMS += \
	SkeletonMainWindow.ui \
	loginDialog.ui \
	manageServersDialog.ui \
	serverDefinitionDialog.ui

RESOURCES = \
	skeleton.qrc

