TEMPLATE = app

TARGET = wolfclient

target.path = $${PREFIX}/bin
INSTALLS += target

CONFIG += qt warn_on

DEFINES += LIBWOLFRAMECLIENT_VISIBILITY=Q_DECL_IMPORT SKELETON_VISIBILITY=Q_DECL_IMPORT X_EXPORT=Q_DECL_IMPORT

QMAKE_RPATHDIR += $$QMAKE_LIBDIR_X11

QMAKE_LIBDIR += ../libqtwolfclient
QMAKE_LIBDIR += ../skeleton
QMAKE_LIBDIR += ../plugins/filechooser
QMAKE_LIBDIR += ../plugins/picturechooser
unix:LIBS += -lqtwolfclient -lskeleton -lfilechooser -lpicturechooser
#unix:LIBS += -L../libqtwolfclient -lqtwolfclient -L../skeleton -lskeleton -L../plugins/filechooser -lfilechooser -L../plugins/picturechooser -lpicturechooser
win32:LIBS += ../libqtwolfclient/debug/qtwolfclient0.lib ../skeleton/debug/skeleton0.lib ../plugins/filechooser/debug/filechooserd.lib ../plugins/picturechooser/debug/picturechooserd.lib
macx:LIBS += ../libqtwolfclient/build/Release/libqtwolfclient0.dylib ../skeleton/build/Release/skeleton0.dylib ../plugins/filechooser/build/Release/libfilechooser.dylib ../plugins/picturechooser/build/Release/libpicturechooser.dylib

INCLUDEPATH += ../libqtwolfclient ../skeleton ../plugins/filechooser ../plugins/picturechooser

#unix:PRE_TARGETDEPS += ../libqtwolfclient/libqtwolfclient.so ../skeleton/libskeleton.so ../plugins/filechooser/libfilechooser.so ../plugins/picturechooser/libpicturechooser.so
unix:QMAKE_LFLAGS += -Wl,-rpath,$$LIBDIR
#unix:QMAKE_LFLAGS += -Wl,-rpath,$$[QT_INSTALL_PLUGINS]/designer
win32:PRE_TARGETDEPS += ../libqtwolfclient/debug/qtwolfclient0.lib ../skeleton/debug/skeleton0.lib ../plugins/filechooser/debug/filechooserd.lib ../plugins/picturechooser/debug/picturechooserd.lib
macx:PRE_TARGETDEPS += ../libqtwolfclient/build/release/libqtwolfclient0.dylib ../skeleton/build/Release/skeleton0.dylib ../plugins/filechooser/build/Release/libfilechooser.dylib ../plugins/picturechooser/build/Release/libpicturechooser.dylib

QT += core gui network

greaterThan(QT_MAJOR_VERSION, 4) {
	QT += widgets
}

greaterThan(QT_MAJOR_VERSION, 4) {
	QT += uitools designer
} else {
	CONFIG += uitools designer
}

SOURCES += \
	MainWindow.cpp \
	FormLoader.cpp \
	FileFormLoader.cpp \
	NetworkFormLoader.cpp \
	DataLoader.cpp \
	FileDataLoader.cpp \
	NetworkDataLoader.cpp \
	WidgetTree.cpp \
	WidgetId.cpp \
	WidgetRequestHeader.cpp \
	WidgetVisitor.cpp \
	WidgetEnabler.cpp \
	WidgetListener.cpp \
	WidgetRequest.cpp \
	WidgetMessageDispatcher.cpp \
	WidgetVisitorStateConstructor.cpp \
	debugview/DebugLogTree.cpp \
	debugview/DebugViewWidget.cpp \
	debugview/DebugHelpers.cpp \
	debugview/DebugMessageHandler.cpp \
	visitors/WidgetVisitor_QLabel.cpp \
	visitors/WidgetVisitor_QComboBox.cpp \
	visitors/WidgetVisitor_QCheckBox.cpp \
	visitors/WidgetVisitor_QGroupBox.cpp \
	visitors/WidgetVisitor_QButtonGroup.cpp \
	visitors/WidgetVisitor_QRadioButton.cpp \
	visitors/WidgetVisitor_QPushButton.cpp \
	visitors/WidgetVisitor_QSpinBox.cpp \
	visitors/WidgetVisitor_QDoubleSpinBox.cpp \
	visitors/WidgetVisitor_QListWidget.cpp \
	visitors/WidgetVisitor_QLineEdit.cpp \
	visitors/WidgetVisitor_QTextEdit.cpp \
	visitors/WidgetVisitor_QPlainTextEdit.cpp \
	visitors/WidgetVisitor_QDateEdit.cpp \
	visitors/WidgetVisitor_QTimeEdit.cpp \
	visitors/WidgetVisitor_QDateTimeEdit.cpp \
	visitors/WidgetVisitor_QTreeWidget.cpp \
	visitors/WidgetVisitor_QTableWidget.cpp \
	visitors/WidgetVisitor_QSlider.cpp \
	visitors/WidgetVisitor_QStackedWidget.cpp \
	visitors/WidgetVisitor_FileChooser.cpp \
	visitors/WidgetVisitor_PictureChooser.cpp \
	serialize/DataFormatXML.cpp \
	serialize/DataStruct.cpp \
	serialize/DataStructDescription.cpp \
	serialize/DataStructDescriptionParse.cpp \
	serialize/DataStructDescriptionMap.cpp \
	serialize/DataStructSerialize.cpp \
	serialize/DataStructMap.cpp \
	serialize/DataPathTree.cpp \
	serialize/DataSerializeItem.cpp \
	serialize/ActionResultDefinition.cpp \
	serialize/ActionDefinition.cpp \
	serialize/CondProperties.cpp \
	FormCall.cpp \
	FormWidget.cpp \
	PreferencesDialog.cpp \
	FormChooseDialog.cpp \
	settings.cpp \
	LoadMode.cpp \
	HistoryLineEdit.cpp \
	wolfclient.cpp

HEADERS += \
	debugview/DebugViewWidget.hpp \
	debugview/DebugLogTree.hpp \
	debugview/DebugHelpers.hpp \
	debugview/DebugMessageHandler.hpp \
	MainWindow.hpp \
	FormLoader.hpp \
	FileFormLoader.hpp \
	NetworkFormLoader.hpp \
	DataLoader.hpp \
	FileDataLoader.hpp \
	NetworkDataLoader.hpp \
	WidgetTree.hpp \
	WidgetVisitorObject.hpp \
	WidgetVisitor.hpp \
	WidgetId.hpp \
	WidgetRequestHeader.hpp \
	WidgetEnabler.hpp \
	WidgetListener.hpp \
	WidgetRequest.hpp \
	WidgetMessageDispatcher.hpp \
	WidgetVisitorStateConstructor.hpp \
	visitors/WidgetVisitor_QLabel.hpp \
	visitors/WidgetVisitor_QComboBox.hpp \
	visitors/WidgetVisitor_QCheckBox.hpp \
	visitors/WidgetVisitor_QGroupBox.hpp \
	visitors/WidgetVisitor_QButtonGroup.hpp \
	visitors/WidgetVisitor_QRadioButton.hpp \
	visitors/WidgetVisitor_QPushButton.hpp \
	visitors/WidgetVisitor_QSpinBox.hpp \
	visitors/WidgetVisitor_QDoubleSpinBox.hpp \
	visitors/WidgetVisitor_QListWidget.hpp \
	visitors/WidgetVisitor_QLineEdit.hpp \
	visitors/WidgetVisitor_QTextEdit.hpp \
	visitors/WidgetVisitor_QPlainTextEdit.hpp \
	visitors/WidgetVisitor_QDateEdit.hpp \
	visitors/WidgetVisitor_QTimeEdit.hpp \
	visitors/WidgetVisitor_QDateTimeEdit.hpp \
	visitors/WidgetVisitor_QTreeWidget.hpp \
	visitors/WidgetVisitor_QTableWidget.hpp \
	visitors/WidgetVisitor_QSlider.hpp \
	visitors/WidgetVisitor_QStackedWidget.hpp \
	visitors/WidgetVisitor_FileChooser.hpp \
	visitors/WidgetVisitor_PictureChooser.hpp \
	serialize/DataFormatXML.hpp \
	serialize/DataStruct.hpp \
	serialize/DataStructDescription.hpp \
	serialize/DataStructDescriptionMap.hpp \
	serialize/DataStructSerialize.hpp \
	serialize/DataStructMap.hpp \
	serialize/DataPathTree.hpp \
	serialize/DataSerializeItem.hpp \
	serialize/VisitorInterface.hpp \
	serialize/ActionResultDefinition.hpp \
	serialize/ActionDefinition.hpp \
	serialize/CondProperties.hpp \
	FormCall.hpp \
	FormWidget.hpp \
	PreferencesDialog.hpp \
	FormChooseDialog.hpp \
	settings.hpp \
	LoadMode.hpp \
	HistoryLineEdit.hpp

win32:RC_FILE=wolfclient.rc

RESOURCES = \
	wolfclient.qrc

FORMS += \
	MainWindow.ui \
	PreferencesDialog.ui \
	PreferencesDialogDeveloper.ui \
	PreferencesDialogInterface.ui

TRANSLATIONS += \
	i18n/wolfclient.de_CH.ts \
	i18n/wolfclient.ro_RO.ts

isEmpty(QMAKE_LRELEASE) {
	win32: QMAKE_LRELEASE = $$[QT_INSTALL_BINS]\\lrelease.exe
	else: { 
		QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease
		!exists($$QMAKE_LRELEASE) { QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease-qt4 }
	}
}
updateqm.input = TRANSLATIONS
updateqm.output = ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.qm
updateqm.commands = $$QMAKE_LRELEASE ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.qm
updateqm.CONFIG += no_link
QMAKE_EXTRA_COMPILERS += updateqm
PRE_TARGETDEPS += compiler_updateqm_make_all

test.depends = first

QMAKE_EXTRA_TARGETS += test
