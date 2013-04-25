TEMPLATE = lib

TARGET = pictureselector

CONFIG += qt warn_on plugin

DEFINES += X_EXPORT=Q_DECL_EXPORT BUILD_AS_PLUGIN

INCLUDEPATH += .

QT += 

greaterThan(QT_MAJOR_VERSION, 4) {
	QT += widgets
}

greaterThan(QT_MAJOR_VERSION, 4) {
	QT += designer
} else {
	CONFIG += designer
}

win32|mac: CONFIG+= debug_and_release
QTDIR_build:DESTDIR = $$QT_BUILD_TREE/plugins/designer
contains(TEMPLATE, ".*lib"):TARGET = $$qtLibraryTarget($$TARGET)

# install
target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS += target

build_all:!build_pass {
    CONFIG -= build_all
    CONFIG += release
}

# Input
SOURCES += \
	ImageSelectionDialog.cpp

HEADERS += \
	ImageSelectionDialog.hpp
