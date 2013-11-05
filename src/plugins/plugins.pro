include(../../qtcreator.pri)

TEMPLATE  = subdirs

SUBDIRS   = \
    coreplugin \
    find \
    texteditor \
    cppeditor \
    bineditor \
    diffeditor \
    imageviewer \
    bookmarks \
    projectexplorer \
    vcsbase \
    perforce \
    subversion \
    git \
    cvs \
    cpptools \
    qtsupport \
    qmakeprojectmanager \
    locator \
    debugger \
    help \
    cpaster \
    cmakeprojectmanager \
    autotoolsprojectmanager \
    fakevim \
    designer \
    resourceeditor \
    genericprojectmanager \
    qmljseditor \
    qmlprojectmanager \
    glsleditor \
    pythoneditor \
    mercurial \
    bazaar \
    classview \
    tasklist \
    analyzerbase \
    qmljstools \
    macros \
    remotelinux \
    android \
    valgrind \
    todo \
    qnx \
    clearcase \
    baremetal \
    ios

# prefer qmake variable set on command line over env var
isEmpty(LLVM_INSTALL_DIR):LLVM_INSTALL_DIR=$$(LLVM_INSTALL_DIR)
!isEmpty(LLVM_INSTALL_DIR) {
    SUBDIRS += clangcodemodel
}

isEmpty(QBS_INSTALL_DIR): QBS_INSTALL_DIR = $$(QBS_INSTALL_DIR)
exists(../shared/qbs/qbs.pro)|!isEmpty(QBS_INSTALL_DIR): \
    SUBDIRS += \
        qbsprojectmanager


isEmpty(IDE_PACKAGE_MODE) {
    SUBDIRS += \
        helloworld \
        updateinfo
} else:!isEmpty(UPDATEINFO_ENABLE) {
    SUBDIRS += \
        updateinfo
}

minQtVersion(5, 2, 0) {
    SUBDIRS += \
        qmldesigner \
        qmlprofiler \
        welcome
} else {
     warning("QmlDesigner plugin has been disabled.")
     warning("QmlProfiler plugin has been disabled.")
     warning("Welcome plugin has been disabled.")
     warning("These plugins need at least Qt 5.2.")
}

for(p, SUBDIRS) {
    QTC_PLUGIN_DEPENDS =
    include($$p/$${p}_dependencies.pri)
    pv = $${p}.depends
    $$pv = $$QTC_PLUGIN_DEPENDS
}

SUBDIRS += debugger/dumper.pro
linux-* {
     SUBDIRS += debugger/ptracepreload.pro
}

include (debugger/lldblib/guest/qtcreator-lldb.pri)

plugin_pythoneditor.subdir = pythoneditor
plugin_pythoneditor.depends = plugin_coreplugin
plugin_pythoneditor.depends += plugin_cpptools
plugin_pythoneditor.depends += plugin_texteditor
