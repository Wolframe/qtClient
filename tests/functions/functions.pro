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

test.target = test
test.commands = ./testDataTree -v2
test.depends = all

QMAKE_EXTRA_TARGETS += test
