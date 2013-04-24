TEMPLATE = app

TARGET = qtSkeleton

CONFIG += thread qt uitools designer debug

DEFINES += LIBWOLFRAMECLIENT_VISIBILITY=Q_DECL_IMPORT SKELETON_VISIBILITY=Q_DECL_IMPORT X_EXPORT=Q_DECL_IMPORT

unix:LIBS += -L../libqtwolframeclient -lqtwolframeclient -L../skeleton -lskeleton -L../plugins/filechooser -lfilechooser -L../plugins/picturechooser -lpicturechooser
win32:LIBS += ../libqtwolframeclient/debug/qtwolframeclient0.lib ../skeleton/debug/skeleton0.lib ../plugins/filechooser/debug/filechooserd.lib ../plugins/picturechooser/debug/picturechooserd.lib
macx:LIBS += ../libqtwolframeclient/build/Release/libqtwolframeclient0.dylib .../skeleton/build/Release/skeleton0.dylib ./plugins/filechooser/build/Release/libfilechooser.dylib ../plugins/picturechooser/build/Release/libpicturechooser.dylib

INCLUDEPATH += ../skeleton ../libqtwolframeclient ../plugins/filechooser ../plugins/picturechooser

unix:PRE_TARGETDEPS += ../libqtwolframeclient/libqtwolframeclient.so ../skeleton/libskeleton.so ../plugins/filechooser/libfilechooser.so ../plugins/picturechooser/libpicturechooser.so
win32:PRE_TARGETDEPS += ../libqtwolframeclient/debug/qtwolframeclient0.lib ../skeleton/debug/skeleton0.lib ../plugins/filechooser/debug/filechooserd.lib ../plugins/picturechooser/debug/picturechooserd.lib
macx:PRE_TARGETDEPS += ../libqtwolframeclient/build/release/libqtwolframeclient0.dylib ..skeleton/build/release/skeleton0.dylib ../plugins/filechooser/build/Release/libfilechooser.dylib ../plugins/picturechooser/build/Release/libpicturechooser.dylib

QT += core gui network

SOURCES += \
	MainWindow.cpp \
	settings.cpp \
	LoadMode.cpp \
	qtSkeleton.cpp

HEADERS += \
	MainWindow.hpp \
	settings.hpp \
	LoadMode.hpp

RESOURCES = \
	qtSkeleton.qrc

FORMS += \
	MainWindow.ui

TRANSLATIONS += \
	i18n/qtSkeleton.de_CH.ts \
	i18n/qtSkeleton.ro_RO.ts

isEmpty(QMAKE_LRELEASE) {
    win32:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]\lrelease.exe
    else:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease
}
updateqm.input = TRANSLATIONS
updateqm.output = ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.qm
updateqm.commands = $$QMAKE_LRELEASE ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.qm
updateqm.CONFIG += no_link
QMAKE_EXTRA_COMPILERS += updateqm
PRE_TARGETDEPS += compiler_updateqm_make_all
