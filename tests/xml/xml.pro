TEMPLATE = app

TARGET = testXmlSerialization

CONFIG += qt warn_on

greaterThan(QT_MAJOR_VERSION, 4) {
	QT += testlib
} else {
	CONFIG += qtestlib
}

INCLUDEPATH += ../../wolfclient

QT += core gui

contains(QT_VERSION,^5\\..*) {
QT += widgets
}

SOURCES += \	
	testXmlSerialization.cpp \
	../../wolfclient/DataFormatXML.cpp

HEADERS += \
	testXmlSerialization.hpp

unix|macx:test.commands = ./testXmlSerialization -v2
#missing QtTest4.dll and others
#win32:debug:test.commands = Debug/testXmlSerialization -v2
#win32:release:test.commands = Release/testXmlSerialization -v2
test.depends = first

QMAKE_EXTRA_TARGETS += test
