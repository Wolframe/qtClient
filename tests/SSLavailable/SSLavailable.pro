TEMPLATE = app

TARGET = SSLavailable

greaterThan( QT_MAJOR_VERSION, 5 )	{
	QT += testlib
}
else	{
	CONFIG += qtestlib
}

QT += network

SOURCES += \
	SSLavailable.cpp
