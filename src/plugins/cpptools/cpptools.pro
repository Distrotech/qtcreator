TEMPLATE = lib
TARGET = CppTools
include(../../qtcreatorplugin.pri)
include($$IDE_SOURCE_TREE/src/plugins/locator/locator.pri)
include(cpptools_dependencies.pri)

# DEFINES += QT_NO_CAST_FROM_ASCII
DEFINES += QT_NO_CAST_TO_ASCII
INCLUDEPATH += .
DEFINES += CPPTOOLS_LIBRARY
HEADERS += completionsettingspage.h \
    cppclassesfilter.h \
    cppcurrentdocumentfilter.h \
    cppfunctionsfilter.h \
    cppmodelmanager.h \
    cpplocatorfilter.h \
    cpptools_global.h \
    cpptoolsconstants.h \
    cpptoolseditorsupport.h \
    cpptoolsplugin.h \
    cppqtstyleindenter.h \
    searchsymbols.h \
    cppdoxygen.h \
    cppfilesettingspage.h \
    cppfindreferences.h \
    cppcodeformatter.h \
    symbolsfindfilter.h \
    uicodecompletionsupport.h \
    insertionpointlocator.h \
    cpprefactoringchanges.h \
    abstracteditorsupport.h \
    cppcompletionassist.h \
    cppcodestylesettingspage.h \
    cpptoolssettings.h \
    cppcodestylesettings.h \
    cppcodestylepreferencesfactory.h \
    cppcodestylepreferences.h \
    cpptoolsreuse.h \
    completionprojectsettingspage.h \
    completionprojectsettings.h \
    pchmanager.h \
    cppctordtorfilter.h \
    doxygengenerator.h \
    commentssettings.h \
    symbolfinder.h \
    cppcompletionsupport.h \
    cpphighlightingsupport.h \
    cppchecksymbols.h \
    cpplocalsymbols.h \
    cppsemanticinfo.h \
    cpphighlightingsupport_p.h

SOURCES += completionsettingspage.cpp \
    cppclassesfilter.cpp \
    cppcurrentdocumentfilter.cpp \
    cppfunctionsfilter.cpp \
    cppmodelmanager.cpp \
    cpplocatorfilter.cpp \
    cpptoolseditorsupport.cpp \
    cpptoolsplugin.cpp \
    cppqtstyleindenter.cpp \
    searchsymbols.cpp \
    cppdoxygen.cpp \
    cppfilesettingspage.cpp \
    abstracteditorsupport.cpp \
    cppfindreferences.cpp \
    cppcodeformatter.cpp \
    symbolsfindfilter.cpp \
    uicodecompletionsupport.cpp \
    insertionpointlocator.cpp \
    cpprefactoringchanges.cpp \
    cppcompletionassist.cpp \
    cppcodestylesettingspage.cpp \
    cpptoolssettings.cpp \
    cppcodestylesettings.cpp \
    cppcodestylepreferencesfactory.cpp \
    cppcodestylepreferences.cpp \
    cpptoolsreuse.cpp \
    completionprojectsettingspage.cpp \
    completionprojectsettings.cpp \
    pchmanager.cpp \
    cppctordtorfilter.cpp \
    doxygengenerator.cpp \
    commentssettings.cpp \
    symbolfinder.cpp \
    cppcompletionsupport.cpp \
    cpphighlightingsupport.cpp \
    cppchecksymbols.cpp \
    cpplocalsymbols.cpp \
    cppsemanticinfo.cpp

FORMS += completionsettingspage.ui \
    cppfilesettingspage.ui \
    cppcodestylesettingspage.ui \
    completionprojectsettingspage.ui

# For Clang integration:
contains(DEFINES, CLANG_COMPLETION) {
    HEADERS += clangcompletion.h
    SOURCES += clangcompletion.cpp
}

contains(DEFINES, CLANG_HIGHLIGHTING) {
    HEADERS += cppcreatemarkers.h
    SOURCES += cppcreatemarkers.cpp
}

HEADERS += clangutils.h
SOURCES += clangutils.cpp

equals(TEST, 1) {
    SOURCES += \
        cppcodegen_test.cpp
}
