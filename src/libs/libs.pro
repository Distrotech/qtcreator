TEMPLATE  = subdirs
CONFIG   += ordered
QT += core gui

SUBDIRS   = \
    3rdparty \
    qtconcurrent \
    aggregation \
    extensionsystem \
    utils \
    utils/process_stub.pro \
    languageutils \
    symbianutils \
    cplusplus \
    qmljs \
    qmljsdebugclient \
    glsl \
    qmleditorwidgets \
    qtcomponents/styleitem \
    clangwrapper
win32:SUBDIRS += utils/process_ctrlc_stub.pro

# Windows: Compile Qt Creator CDB extension if Debugging tools can be detected.    
win32 {
    include(qtcreatorcdbext/cdb_detect.pri)
    exists($$CDB_PATH):SUBDIRS += qtcreatorcdbext
}
