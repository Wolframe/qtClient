TEMPLATE = lib

TARGET = qtwolframeclient

isEmpty(LIBDIR) {
  LIBDIR = $${PREFIX}/lib
}

target.path = $$LIBDIR
INSTALLS += target

VERSION = 0.0.1

CONFIG += qt warn_on

DEFINES += LIBWOLFRAMECLIENT_VISIBILITY=Q_DECL_EXPORT WITH_SSL=1

QT += core network

contains(QT_VERSION,^5\\..*) {
QT += widgets
}

SOURCES += \
	WolframeClient.cpp \
	WolframeClientProtocolBase.cpp \
	WolframeClientProtocol.cpp \
	serverDefinition.cpp \
	serverConnection.cpp

HEADERS += \
	WolframeClient.hpp \
	private/WolframeClientProtocolBase.hpp \
	private/WolframeClientProtocol.hpp \
	serverDefinition.hpp \
	serverConnection.hpp

test.depends = first

QMAKE_EXTRA_TARGETS += test
