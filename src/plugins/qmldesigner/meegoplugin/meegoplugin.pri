TARGET = meegoplugin
TEMPLATE = lib
CONFIG += plugin

include (../designercore/iwidgetplugin.pri)

SOURCES += $$PWD/meegoplugin.cpp

HEADERS += $$PWD/meegoplugin.h  $$PWD/../designercore/include/iwidgetplugin.h

RESOURCES += $$PWD/meegoplugin.qrc

OTHER_FILES += $$PWD/meego.metainfo
