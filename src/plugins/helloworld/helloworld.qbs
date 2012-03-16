import qbs.base 1.0

import "../QtcPlugin.qbs" as QtcPlugin

QtcPlugin {
    name: "HelloWorld"

    Depends { name: "Core" }
    Depends { name: "cpp" }
    Depends { name: "qt"; submodules: ['gui', 'xml', 'network', 'script'] }

    cpp.includePaths: [
        "..",
        "../../libs",
        buildDirectory
    ]

    files: [
        "helloworldplugin.h",
        "helloworldwindow.h",
        "helloworldplugin.cpp",
        "helloworldwindow.cpp"
    ]
}

