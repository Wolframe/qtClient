HEADERS = \
	pluginWindow.hpp
SOURCES =   main.cpp \
	pluginWindow.cpp

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../build-qtClient-Desktop_qt4-Debug/plugins/wimagelistwidget/release/ -lwimagelistwidget
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../build-qtClient-Desktop_qt4-Debug/plugins/wimagelistwidget/debug/ -lwimagelistwidget
else:unix: LIBS += -L$$PWD/../../../build-qtClient-Desktop_qt4-Debug/plugins/wimagelistwidget/ -lwimagelistwidget

INCLUDEPATH += $$PWD/../../plugins/wimagelistwidget
DEPENDPATH += $$PWD/../../plugins/wimagelistwidget
