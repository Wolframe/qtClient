TEMPLATE = app

TARGET = skeletonTest

CONFIG += qt warn_on

DEFINES += LIBWOLFRAMECLIENT_VISIBILITY=Q_DECL_IMPORT SKELETON_VISIBILITY=Q_DECL_IMPORT

QT += core network

contains(QT_VERSION,^5\\..*) {
QT += widgets
}

unix:LIBS += -L../../libqtwolframeclient -lqtwolframeclient -L../../skeleton -lskeleton
win32:LIBS += ../../libqtwolframeclient/debug/qtwolframeclient0.lib ../../skeleton/debug/skeleton0.lib

INCLUDEPATH += ../../libqtwolframeclient ../../skeleton

unix:PRE_TARGETDEPS += ../../libqtwolframeclient/libqtwolframeclient.so ../../skeleton/libskeleton.so
win32:PRE_TARGETDEPS += ../../libqtwolframeclient/debug/qtwolframeclient0.lib ../../skeleton/debug/skeleton0.lib

SOURCES += \
	main.cpp

RESOURCES = \
	skeletonTest.qrc

test.target = test
test.commands =
test.depends = 

QMAKE_EXTRA_TARGETS += test

