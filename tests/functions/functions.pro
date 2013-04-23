TEMPLATE = app

TARGET = testDataTree

CONFIG += qt debug

INCLUDEPATH += ../../qtclient

QT += core gui

contains(QT_VERSION,^5\\..*) {
QT += widgets
}

SOURCES += \	
	testDataTree.cpp \
	../../qtclient/DataTree.cpp
