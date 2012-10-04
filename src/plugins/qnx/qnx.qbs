import qbs.base 1.0

import "../QtcPlugin.qbs" as QtcPlugin

QtcPlugin {
    name: "Qnx"
    provider: "RIM"

    Depends { name: "Core" }
    Depends { name: "Debugger" }
    Depends { name: "ProjectExplorer" }
    Depends { name: "QtSupport" }
    Depends { name: "Qt4ProjectManager" }
    Depends { name: "RemoteLinux" }
    Depends { name: "Qt"; submodules: ["widgets", "xml", "network"] }

    property bool enable: false
    property var pluginspecreplacements: ({"QNX_EXPERIMENTAL_STR": (enable ? "false" : "true")})

    cpp.defines: base.concat(["QT_NO_CAST_TO_ASCII", "QT_NO_CAST_FROM_ASCII"])
    cpp.includePaths: [
        "..",
        buildDirectory,
        "../../libs",
        "../../shared"
    ]

    files: [
        "bardescriptorfileimagewizardpage.cpp",
        "bardescriptorfileimagewizardpage.h",
        "bardescriptorfileimagewizardpage.ui",
        "blackberryabstractdeploystep.cpp",
        "blackberryabstractdeploystep.h",
        "blackberryapplicationrunner.cpp",
        "blackberryapplicationrunner.h",
        "blackberryconnect.cpp",
        "blackberryconnect.h",
        "blackberrycreatepackagestepconfigwidget.cpp",
        "blackberrycreatepackagestepconfigwidget.h",
        "blackberrycreatepackagestep.cpp",
        "blackberrycreatepackagestepfactory.cpp",
        "blackberrycreatepackagestepfactory.h",
        "blackberrycreatepackagestep.h",
        "blackberrydebugsupport.cpp",
        "blackberrydebugsupport.h",
        "blackberrydeployconfiguration.cpp",
        "blackberrydeployconfigurationfactory.cpp",
        "blackberrydeployconfigurationfactory.h",
        "blackberrydeployconfiguration.h",
        "blackberrydeployconfigurationwidget.cpp",
        "blackberrydeployconfigurationwidget.h",
        "blackberrydeployconfigurationwidget.ui",
        "blackberrydeployinformation.cpp",
        "blackberrydeployinformation.h",
        "blackberrydeploystepconfigwidget.cpp",
        "blackberrydeploystepconfigwidget.h",
        "blackberrydeploystep.cpp",
        "blackberrydeploystepfactory.cpp",
        "blackberrydeploystepfactory.h",
        "blackberrydeploystep.h",
        "blackberrydeviceconfiguration.cpp",
        "blackberrydeviceconfigurationfactory.cpp",
        "blackberrydeviceconfigurationfactory.h",
        "blackberrydeviceconfiguration.h",
        "blackberrydeviceconfigurationwidget.cpp",
        "blackberrydeviceconfigurationwidget.h",
        "blackberrydeviceconfigurationwidget.ui",
        "blackberrydeviceconfigurationwizard.cpp",
        "blackberrydeviceconfigurationwizard.h",
        "blackberrydeviceconfigurationwizardpages.cpp",
        "blackberrydeviceconfigurationwizardpages.h",
        "blackberrydeviceconfigurationwizardsetuppage.ui",
        "blackberrydeviceconfigurationwizardsshkeypage.ui",
        "blackberryqtversion.cpp",
        "blackberryqtversionfactory.cpp",
        "blackberryqtversionfactory.h",
        "blackberryqtversion.h",
        "blackberryrunconfiguration.cpp",
        "blackberryrunconfigurationfactory.cpp",
        "blackberryrunconfigurationfactory.h",
        "blackberryrunconfiguration.h",
        "blackberryrunconfigurationwidget.cpp",
        "blackberryrunconfigurationwidget.h",
        "blackberryrunconfigurationwidget.ui",
        "blackberryruncontrol.cpp",
        "blackberryruncontrolfactory.cpp",
        "blackberryruncontrolfactory.h",
        "blackberryruncontrol.h",
        "blackberrywizardextension.cpp",
        "blackberrywizardextension.h",
        "images/target.png",
        "images/target-small.png",
        "pathchooserdelegate.cpp",
        "pathchooserdelegate.h",
        "qnxabstractqtversion.cpp",
        "qnxabstractqtversion.h",
        "qnxbaseqtconfigwidget.cpp",
        "qnxbaseqtconfigwidget.h",
        "qnxbaseqtconfigwidget.ui",
        "qnxconstants.h",
        "qnxdebugsupport.cpp",
        "qnxdebugsupport.h",
        "qnxdeployconfiguration.cpp",
        "qnxdeployconfigurationfactory.cpp",
        "qnxdeployconfigurationfactory.h",
        "qnxdeployconfiguration.h",
        "qnxdeploystepfactory.cpp",
        "qnxdeploystepfactory.h",
        "qnxdeviceconfiguration.cpp",
        "qnxdeviceconfigurationfactory.cpp",
        "qnxdeviceconfigurationfactory.h",
        "qnxdeviceconfiguration.h",
        "qnxdeviceconfigurationwizard.cpp",
        "qnxdeviceconfigurationwizard.h",
        "qnxdeviceconfigurationwizardpages.cpp",
        "qnxdeviceconfigurationwizardpages.h",
        "qnxplugin.cpp",
        "qnxplugin.h",
        "qnx.qrc",
        "qnxqtversion.cpp",
        "qnxqtversionfactory.cpp",
        "qnxqtversionfactory.h",
        "qnxqtversion.h",
        "qnxrunconfiguration.cpp",
        "qnxrunconfigurationfactory.cpp",
        "qnxrunconfigurationfactory.h",
        "qnxrunconfiguration.h",
        "qnxruncontrol.cpp",
        "qnxruncontrolfactory.cpp",
        "qnxruncontrolfactory.h",
        "qnxruncontrol.h",
        "qnxutils.cpp",
        "qnxutils.h"
    ]
}
