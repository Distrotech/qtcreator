import qbs.base 1.0

import "../QtcPlugin.qbs" as QtcPlugin

QtcPlugin {
    name: "QtSupport"

    Depends { name: "qt"; submodules: ['gui', 'declarative'] }
    Depends { name: "Core" }
    Depends { name: "ProjectExplorer" }
    Depends { name: "TextEditor" }
    Depends { name: "QmlJS" }

    Depends { name: "cpp" }
    cpp.includePaths: [
        ".",
        "../../shared",
        "../../shared/proparser",
        "..",
        "../../libs",
        buildDirectory
    ]
    cpp.defines: {
        return base.concat([
            "QT_NO_CAST_TO_ASCII",
            "PROPARSER_AS_LIBRARY",
            "PROPARSER_LIBRARY",
            "PROPARSER_THREAD_SAFE",
            "PROEVALUATOR_THREAD_SAFE",
            "PROEVALUATOR_CUMULATIVE"
        ])
    }

    files: [
        "../../shared/proparser/proparser_global.h",
        "../../shared/proparser/profileparser.h",
        "../../shared/proparser/profileevaluator.h",
        "../../shared/proparser/proitems.h",
        "../../shared/proparser/prowriter.h",
        "../../shared/proparser/profileparser.cpp",
        "../../shared/proparser/profileevaluator.cpp",
        "../../shared/proparser/proitems.cpp",
        "../../shared/proparser/prowriter.cpp",
        "../../shared/proparser/proparser.qrc",
        "../../shared/proparser/ioutils.h",
        "../../shared/proparser/ioutils.cpp",
        "qtversioninfo.ui",
        "qtversionmanager.ui",
        "baseqtversion.h",
        "debugginghelper.ui",
        "debugginghelperbuildtask.h",
        "exampleslistmodel.h",
        "gettingstartedwelcomepage.h",
        "profilereader.cpp",
        "profilereader.h",
        "qmldebugginglibrary.h",
        "qmldumptool.h",
        "qmlobservertool.h",
        "qtoptionspage.h",
        "qtoutputformatter.cpp",
        "qtoutputformatter.h",
        "qtparser.h",
        "qtsupport_global.h",
        "qtsupportconstants.h",
        "qtsupportplugin.cpp",
        "qtsupportplugin.h",
        "qtversionfactory.h",
        "qtversionmanager.h",
        "screenshotcropper.cpp",
        "screenshotcropper.h",
        "showbuildlog.ui",
        "baseqtversion.cpp",
        "debugginghelperbuildtask.cpp",
        "exampleslistmodel.cpp",
        "gettingstartedwelcomepage.cpp",
        "qmldebugginglibrary.cpp",
        "qmldumptool.cpp",
        "qmlobservertool.cpp",
        "qtoptionspage.cpp",
        "qtparser.cpp",
        "qtversionfactory.cpp",
        "qtversionmanager.cpp"
    ]
}

