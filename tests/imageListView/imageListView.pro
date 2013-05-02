HEADERS = \
	pluginWindow.hpp
SOURCES =   main.cpp \
	pluginWindow.cpp

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../build-qtClient-Desktop_qt4-Debug/plugins/imagelistview/release/ -limagelistview
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../build-qtClient-Desktop_qt4-Debug/plugins/imagelistview/debug/ -limagelistview
else:unix: LIBS += -L$$PWD/../../../build-qtClient-Desktop_qt4-Debug/plugins/imagelistview/ -limagelistview

INCLUDEPATH += $$PWD/../../plugins/imagelistview
DEPENDPATH += $$PWD/../../plugins/imagelistview
