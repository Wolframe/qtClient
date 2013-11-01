TEMPLATE = lib

TARGET = filechooser

CONFIG += qt warn_on plugin

DEFINES += X_EXPORT=Q_DECL_EXPORT BUILD_AS_PLUGIN

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

# only because wolfclient links in visitors directly to the plugins
# TODO: must be solved later, so far we just deploy the plugins twice:
# as plugin (above) and as shared library (here):
temptarget.files = libfilechooser.so
temptarget.path = $${LIBDIR}
INSTALLS += temptarget

build_all:!build_pass {
    CONFIG -= build_all
    CONFIG += release
}

# Input
SOURCES += \
	FileChooserPlugin.cpp \
	FileChooser.cpp

HEADERS += \
	FileChooserPlugin.hpp \
	FileChooser.hpp
