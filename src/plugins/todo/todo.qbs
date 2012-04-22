import qbs.base 1.0

import "../QtcPlugin.qbs" as QtcPlugin

QtcPlugin {
    name: "Todo"

    Depends { name: "qt"; submodules: ['widgets'] }
    Depends { name: "Core" }
    Depends { name: "TextEditor" }
    Depends { name: "ProjectExplorer" }
    Depends { name: "CPlusPlus" }
    Depends { name: "CppTools" }
    Depends { name: "QmlJs" }

    Depends { name: "cpp" }
    cpp.defines: base.concat(["QT_NO_CAST_TO_ASCII"])
    cpp.includePaths: [
        "..",
        "../../libs",
        buildDirectory
    ]

    files: [
        "constants.h",
        "cpptodoitemsscanner.cpp",
        "cpptodoitemsscanner.h",
        "keyword.cpp",
        "keyworddialog.cpp",
        "keyworddialog.h",
        "keyworddialog.ui",
        "keyword.h",
        "lineparser.cpp",
        "lineparser.h",
        "optionsdialog.cpp",
        "optionsdialog.h",
        "optionsdialog.ui",
        "optionspage.cpp",
        "optionspage.h",
        "qmljstodoitemsscanner.cpp",
        "qmljstodoitemsscanner.h",
        "settings.cpp",
        "settings.h",
        "todoitem.h",
        "todoitemsmodel.cpp",
        "todoitemsmodel.h",
        "todoitemsprovider.cpp",
        "todoitemsprovider.h",
        "todoitemsscanner.cpp",
        "todoitemsscanner.h",
        "todooutputpane.cpp",
        "todooutputpane.h",
        "todoplugin.cpp",
        "todoplugin.h",
        "todoplugin.qrc"
    ]
}
