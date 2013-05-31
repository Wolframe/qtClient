TEMPLATE = lib

TARGET = wolfdesigner

CONFIG += qt warn_on plugin

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
	WolfDesignerPlugin.cpp \
	WolfDesignerMenu.cpp \
	WolfDesignerDialog.cpp
	
HEADERS += \
	WolfDesignerPlugin.hpp \
	WolfDesignerMenu.hpp \
	WolfDesignerDialog.hpp \
	WolfDesigner.hpp

FORMS += \
	WolfDesignerDialog.ui
