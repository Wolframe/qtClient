TEMPLATE = app

TARGET = SSLavailable

greaterThan(QT_MAJOR_VERSION, 4) {
	QT += testlib
} else {
	CONFIG += qtestlib
}

QT += network

SOURCES += \
	SSLavailable.cpp

HEADERS += \
	SSLavailable.hpp

test.target = test
test.commands = ./SSLavailable -v2
test.depends = all

QMAKE_EXTRA_TARGETS += test
