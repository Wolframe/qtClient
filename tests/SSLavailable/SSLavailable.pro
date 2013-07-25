TEMPLATE = app

TARGET = SSLavailable

greaterThan(QT_MAJOR_VERSION, 4)	{
	QT += testlib
}
else	{
	CONFIG += qtestlib
}

QT += network

SOURCES += \
	SSLavailable.cpp

HEADERS += \
	SSLavailable.hpp

unix|macx:test.commands = ./SSLavailable -v2
#missing QtTest4.dll and others
#win32:debug:test.commands = Debug/SSLavailable -v2
#win32:release:test.commands = Release/SSLavailable -v2
test.depends = first

QMAKE_EXTRA_TARGETS += test
