TEMPLATE = app

TARGET = testDataStructMap

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
	testDataStructMap.cpp \
	DataPathTreeVisitor.cpp \
	../../qtclient/serialize/DataStructMap.cpp \
	../../qtclient/serialize/DataPathTree.cpp \
	../../qtclient/serialize/DataSerializeItem.cpp \
	../../qtclient/serialize/DataStruct.cpp \
	../../qtclient/serialize/DataStructDescription.cpp \
	../../qtclient/serialize/DataStructDescriptionMap.cpp \
	../../qtclient/serialize/DataStructDescriptionParse.cpp \
	../../qtclient/serialize/DataStructSerialize.cpp

HEADERS += \
	testDataStructMap.hpp \
	DataPathTreeVisitor.hpp \
	../../qtclient/serialize/DataPathTree.hpp \
	../../qtclient/serialize/DataSerializeItem.hpp \
	../../qtclient/serialize/DataStructDescription.hpp \
	../../qtclient/serialize/DataStructDescriptionMap.hpp \
	../../qtclient/serialize/DataStruct.hpp \
	../../qtclient/serialize/DataStructMap.hpp \
	../../qtclient/serialize/DataStructSerialize.hpp \
	../../qtclient/serialize/VisitorInterface.hpp

unix|macx:test.commands = ./testDataStructMap -v2
#missing QtTest4.dll and others
#win32:debug:test.commands = Debug/testDataStructMap -v2
#win32:release:test.commands = Release/testDataStructMap -v2
test.depends = first

QMAKE_EXTRA_TARGETS += test
