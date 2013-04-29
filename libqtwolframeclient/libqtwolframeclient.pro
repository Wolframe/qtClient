TEMPLATE = lib

TARGET = qtwolframeclient

VERSION = 0.0.1

CONFIG += qt debug static_and_shared

DEFINES += LIBWOLFRAMECLIENT_VISIBILITY=Q_DECL_EXPORT WITH_SSL=1

QT += core network

contains(QT_VERSION,^5\\..*) {
QT += widgets
}

SOURCES += \
	WolframeClient.cpp \
	WolframeClientProtocolBase.cpp \
	WolframeClientProtocol.cpp \
    serverDefinition.cpp

HEADERS += \
	WolframeClient.hpp \
	private/WolframeClientProtocolBase.hpp \
	private/WolframeClientProtocol.hpp \
    serverDefinition.hpp
