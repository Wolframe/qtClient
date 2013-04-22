TEMPLATE = app

TARGET = testDataTree

CONFIG += qt debug

INCLUDEPATH += ../..

QT += core gui

contains(QT_VERSION,^5\\..*) {
QT += widgets
}

SOURCES += \	
	testDataTree.cpp \
	../../DataTree.cpp
