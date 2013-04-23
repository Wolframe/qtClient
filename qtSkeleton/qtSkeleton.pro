TEMPLATE = app

TARGET = qtSkeleton

CONFIG += thread qt uitools designer debug

DEFINES += X_EXPORT=Q_DECL_IMPORT WITH_SSL=1

unix:LIBS += -L../libqtwolframeclient -lqtwolframeclient
win32:LIBS += ../libqtwolframeclient/debug/qtwolframeclient0.lib

INCLUDEPATH += ../libqtwolframeclient

unix:PRE_TARGETDEPS += ../libqtwolframeclient/libqtwolframeclient.so
win32:PRE_TARGETDEPS += ../libqtwolframeclient/debug/qtwolframeclient0.lib

QT += core gui network sql

SOURCES += \
	MainWindow.cpp \
	DataLoader.cpp \
	FileDataLoader.cpp \
	NetworkDataLoader.cpp \
	PreferencesDialog.cpp \
	manageServersDialog.cpp \
	serverDefinitionDialog.cpp \
	settings.cpp \
	loginDialog.cpp \
	LoadMode.cpp \
	qtSkeleton.cpp \
	../skeleton/loginDialog.cpp \
	../skeleton/manageServersDialog.cpp \
	../skeleton/serverDefinitionDialog.cpp

HEADERS += \
	MainWindow.hpp \
	DataLoader.hpp \
	FileDataLoader.hpp \
	NetworkDataLoader.hpp \
	PreferencesDialog.hpp \
	manageServersDialog.hpp \
	serverDefinitionDialog.hpp \
	settings.hpp \
	loginDialog.hpp \
	LoadMode.hpp \
	../skeleton/loginDialog.hpp \
	../skeleton/manageServersDialog.hpp \
	../skeleton/serverDefinitionDialog.hpp

RESOURCES = \
	qtSkeleton.qrc

FORMS += \
	MainWindow.ui \
	PreferencesDialog.ui \
	PreferencesDialogDeveloper.ui \
	PreferencesDialogInterface.ui

TRANSLATIONS += \
	i18n/qtSkeleton.de_CH.ts \
	i18n/qtSkeleton.ro_RO.ts

QT_LRELEASE = $$QMAKE_MOC
QT_LRELEASE ~= s,moc,lrelease,

updateqm.input = TRANSLATIONS
updateqm.output = ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.qm
isEmpty(vcproj):updateqm.variable_out = PRE_TARGETDEPS
updateqm.commands = $$QT_LRELEASE ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_OUT}
updateqm.name = LRELEASE ${QMAKE_FILE_IN}
updateqm.CONFIG += no_link
QMAKE_EXTRA_COMPILERS += updateqm
