import qbs.base 1.0

DynamicLibrary {
    name: "zeroconf"
    destination: "lib"

    Depends { name: "cpp" }
    Depends { name: "Qt.network" }

    cpp.includePaths: "."
    cpp.defines: ["ZEROCONF_LIBRARY"]
    Properties {
        condition: qbs.targetOS == "windows"
        cpp.dynamicLibraries:  "ws2_32"
    }
    Properties {
        condition: qbs.targetOS == "linux"
        cpp.defines: outer.concat([
            "_GNU_SOURCE",
            "HAVE_IPV6",
            "USES_NETLINK",
            "HAVE_LINUX",
            "TARGET_OS_LINUX"
        ])
    }

    files: [
        "servicebrowser.cpp", "servicebrowser.h", "servicebrowser_p.h",
        "embeddedLib.cpp",
        "mdnsderived.cpp", "mdnsderived.h",
        "avahiLib.cpp",
        "dnsSdLib.cpp",
        "dns_sd_types.h",
        "zeroconf_global.h",
        "syssocket.h"
    ]

    ProductModule { cpp.includePaths: "." }
}
