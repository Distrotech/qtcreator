import qbs.base 1.0
import "../QtcLibrary.qbs" as QtcLibrary

QtcLibrary {
    name: "QmlDebug"

    cpp.includePaths: [
        ".",
        ".."
    ]
    cpp.defines: [
        "QMLDEBUG_LIB"
    ]

    Depends { name: "cpp" }
    Depends { name: "Qt.gui" }
    Depends { name: "Qt.network" }
    Depends { name: "symbianutils" }

    files: [
        "baseenginedebugclient.cpp",
        "baseenginedebugclient.h",
        "basetoolsclient.cpp",
        "basetoolsclient.h",
        "declarativeenginedebugclient.h",
        "declarativetoolsclient.cpp",
        "declarativetoolsclient.h",
        "qdebugmessageclient.cpp",
        "qdebugmessageclient.h",
        "qmldebugclient.cpp",
        "qmldebugclient.h",
        "qmldebugconstants.h",
        "qmldebug_global.h",
        "qmloutputparser.cpp",
        "qmloutputparser.h",
        "qmlprofilereventlocation.h",
        "qmlprofilereventtypes.h",
        "qmlprofilertraceclient.cpp",
        "qmlprofilertraceclient.h",
        "qpacketprotocol.cpp",
        "qpacketprotocol.h",
        "qmlenginedebugclient.cpp",
        "qmlenginedebugclient.h",
        "qv8profilerclient.cpp",
        "qv8profilerclient.h",
        "qmltoolsclient.cpp",
        "qmltoolsclient.h"
    ]

    ProductModule {
        Depends { name: "cpp" }
        Depends { name: "symbianutils" }
        cpp.includePaths: [
            "."
        ]
    }
}

