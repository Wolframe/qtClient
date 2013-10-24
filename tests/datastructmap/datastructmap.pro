TEMPLATE = app

TARGET = testDataStructMap

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
	testDataStructMap.cpp \
	DataPathTreeVisitor.cpp \
	../../wolfclient/debugview/DebugHelpers.cpp \
	../../wolfclient/serialize/DataStructMap.cpp \
	../../wolfclient/serialize/DataPathTree.cpp \
	../../wolfclient/serialize/DataSerializeItem.cpp \
	../../wolfclient/serialize/DataStruct.cpp \
	../../wolfclient/serialize/DataStructDescription.cpp \
	../../wolfclient/serialize/DataStructDescriptionMap.cpp \
	../../wolfclient/serialize/DataStructDescriptionParse.cpp \
	../../wolfclient/serialize/DataStructSerialize.cpp

HEADERS += \
	testDataStructMap.hpp \
	DataPathTreeVisitor.hpp \
	../../wolfclient/DebugHelpers.hpp \
	../../wolfclient/serialize/DataPathTree.hpp \
	../../wolfclient/serialize/DataSerializeItem.hpp \
	../../wolfclient/serialize/DataStructDescription.hpp \
	../../wolfclient/serialize/DataStructDescriptionMap.hpp \
	../../wolfclient/serialize/DataStruct.hpp \
	../../wolfclient/serialize/DataStructMap.hpp \
	../../wolfclient/serialize/DataStructSerialize.hpp \
	../../wolfclient/serialize/VisitorInterface.hpp

unix|macx:test.commands = ./testDataStructMap -v2
#missing QtTest4.dll and others
#win32:debug:test.commands = Debug/testDataStructMap -v2
#win32:release:test.commands = Release/testDataStructMap -v2
test.depends = first

QMAKE_EXTRA_TARGETS += test
