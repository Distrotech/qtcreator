VPATH += $$PWD

SOURCES += propertyeditorview.cpp \
    qmlanchorbindingproxy.cpp \
    filewidget.cpp \
    propertyeditorvalue.cpp \
    fontwidget.cpp \
    originwidget.cpp \
    siblingcombobox.cpp \
    propertyeditortransaction.cpp \
    propertyeditorcontextobject.cpp \
    quick2propertyeditorview.cpp \
    gradientlineqmladaptor.cpp \
    propertyeditorqmlbackend.cpp \
    propertyeditorwidget.cpp

HEADERS += propertyeditorview.h \
    qmlanchorbindingproxy.h \
    filewidget.h \
    propertyeditorvalue.h \
    fontwidget.h \
    originwidget.h \
    siblingcombobox.h \
    propertyeditortransaction.h \
    designerpropertymap.h \
    propertyeditorcontextobject.h \
    quick2propertyeditorview.h \
    gradientlineqmladaptor.h \
    propertyeditorqmlbackend.h \
    propertyeditorwidget.h
    
QT += qml quick

RESOURCES += propertyeditor.qrc
