TEMPLATE = app

TARGET = testDataTree

CONFIG += qt warn_on

greaterThan(QT_MAJOR_VERSION, 4) {
	QT += testlib
} else {
	CONFIG += qtestlib
}

INCLUDEPATH += ../../qtclient

QT += core gui

contains(QT_VERSION,^5\\..*) {
QT += widgets
}

SOURCES += \	
	testDataTree.cpp \
	../../qtclient/DataTree.cpp

HEADERS += \
	testDataTree.hpp

unix|macx:test.commands = ./testDataTree -v2
#missing QtTest4.dll and others
#win32:debug:test.commands = Debug/testDataTree -v2
#win32:release:test.commands = Release/testDataTree -v2
test.depends = first

QMAKE_EXTRA_TARGETS += test
