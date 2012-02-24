import qbs.base 1.0

DynamicLibrary {
    name: "symbianutils"
    destination: "lib"

    cpp.includePaths: [
        ".",
        "..",
        "../../shared/symbianutils",
        "../../shared/json"
    ]
    cpp.defines: [
        "SYMBIANUTILS_BUILD_LIB",
        "HAS_SERIALPORT"
    ]
    cpp.optimization: "fast"

    Depends { name: "cpp" }
    Depends { name: "Qt"; submodules: ["network"]}
    Depends { name: "utils" }

    Group {
        condition: qbs.targetOS == "linux" || qbs.targetOS == "mac"
        files: [
            "../../shared/symbianutils/virtualserialdevice_posix.cpp"
        ]
    }

    Group {
        condition: qbs.targetOS == "windows"
        files: [
            "../../shared/symbianutils/virtualserialdevice_win.cpp"
        ]
    }

    files: [
        "../../shared/symbianutils/callback.h",
        "../../shared/symbianutils/codadevice.cpp",
        "../../shared/symbianutils/codadevice.h",
        "../../shared/symbianutils/codamessage.cpp",
        "../../shared/symbianutils/codamessage.h",
        "../../shared/symbianutils/codautils.cpp",
        "../../shared/symbianutils/codautils.h",
        "../../shared/symbianutils/codautils_p.h",
        "../../shared/symbianutils/symbiandevicemanager.cpp",
        "../../shared/symbianutils/symbiandevicemanager.h",
        "../../shared/symbianutils/symbianutils_global.h",
        "../../shared/symbianutils/virtualserialdevice.cpp",
        "../../shared/symbianutils/virtualserialdevice.h",
        "../../shared/json/json_global.h",
        "../../shared/json/json.h",
        "../../shared/json/json.cpp"
    ]

    ProductModule {
        cpp.includePaths: [
            "../../shared/symbianutils",
            "../../shared/json",
            "."
        ]
    }
}

