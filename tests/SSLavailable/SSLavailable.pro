TEMPLATE = app

TARGET = SSLavailable

CONFIG += qtestlib

QT += network

SOURCES += \
	SSLavailable.cpp

HEADERS += \
	SSLavailable.hpp

test.target = test
test.commands = ./SSLavailable -v2
test.depends = all

QMAKE_EXTRA_TARGETS += test
