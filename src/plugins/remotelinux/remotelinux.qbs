import qbs.base 1.0

import "../QtcPlugin.qbs" as QtcPlugin

QtcPlugin {
    name: "RemoteLinux"

    Depends { name: "Qt.widgets" }
    Depends { name: "Core" }
    Depends { name: "Debugger" }
    Depends { name: "ProjectExplorer" }
    Depends { name: "QtSupport" }
    Depends { name: "QtcSsh" }

    Depends { name: "cpp" }
    cpp.includePaths: [
        "..",
        "../../libs",
        buildDirectory
    ]

    files: [
        "abstractpackagingstep.cpp",
        "abstractpackagingstep.h",
        "abstractremotelinuxdeployservice.cpp",
        "abstractremotelinuxdeployservice.h",
        "abstractremotelinuxdeploystep.h",
        "abstractuploadandinstallpackageservice.cpp",
        "abstractuploadandinstallpackageservice.h",
        "embeddedlinuxqtversion.cpp",
        "embeddedlinuxqtversion.h",
        "embeddedlinuxqtversionfactory.cpp",
        "embeddedlinuxqtversionfactory.h",
        "genericdirectuploadservice.cpp",
        "genericdirectuploadstep.h",
        "genericlinuxdeviceconfigurationfactory.cpp",
        "genericlinuxdeviceconfigurationfactory.h",
        "genericlinuxdeviceconfigurationwizard.cpp",
        "genericlinuxdeviceconfigurationwizard.h",
        "genericlinuxdeviceconfigurationwizardpages.cpp",
        "genericlinuxdeviceconfigurationwizardpages.h",
        "genericlinuxdeviceconfigurationwizardsetuppage.ui",
        "genericlinuxdeviceconfigurationwidget.cpp",
        "genericlinuxdeviceconfigurationwidget.h",
        "genericlinuxdeviceconfigurationwidget.ui",
        "genericremotelinuxdeploystepfactory.cpp",
        "genericremotelinuxdeploystepfactory.h",
        "linuxdevice.cpp",
        "linuxdevice.h",
        "linuxdevicetestdialog.cpp",
        "linuxdevicetestdialog.h",
        "linuxdevicetester.cpp",
        "linuxdevicetester.h",
        "publickeydeploymentdialog.cpp",
        "publickeydeploymentdialog.h",
        "remotelinux.qrc",
        "remotelinux_constants.h",
        "remotelinux_export.h",
        "remotelinuxcustomcommanddeploymentstep.h",
        "remotelinuxcustomcommanddeployservice.cpp",
        "remotelinuxcustomcommanddeployservice.h",
        "remotelinuxdebugsupport.cpp",
        "remotelinuxdebugsupport.h",
        "remotelinuxdeployconfiguration.cpp",
        "remotelinuxdeployconfiguration.h",
        "remotelinuxdeployconfigurationfactory.cpp",
        "remotelinuxdeployconfigurationfactory.h",
        "remotelinuxdeployconfigurationwidget.cpp",
        "remotelinuxdeployconfigurationwidget.h",
        "remotelinuxpackageinstaller.cpp",
        "remotelinuxplugin.cpp",
        "remotelinuxplugin.h",
        "remotelinuxrunconfiguration.cpp",
        "remotelinuxrunconfiguration.h",
        "remotelinuxrunconfigurationfactory.cpp",
        "remotelinuxrunconfigurationfactory.h",
        "remotelinuxrunconfigurationwidget.cpp",
        "remotelinuxrunconfigurationwidget.h",
        "remotelinuxruncontrol.h",
        "remotelinuxutils.cpp",
        "remotelinuxutils.h",
        "tarpackagecreationstep.h",
        "uploadandinstalltarpackagestep.h",
        "genericdirectuploadservice.h",
        "linuxdevicetestdialog.ui",
        "packageuploader.cpp",
        "packageuploader.h",
        "remotelinuxdeployconfigurationwidget.ui",
        "remotelinuxenvironmentreader.cpp",
        "remotelinuxenvironmentreader.h",
        "remotelinuxpackageinstaller.h",
        "sshkeydeployer.cpp",
        "sshkeydeployer.h",
        "typespecificdeviceconfigurationlistmodel.cpp",
        "typespecificdeviceconfigurationlistmodel.h",
        "abstractremotelinuxdeploystep.cpp",
        "genericdirectuploadstep.cpp",
        "remotelinuxcustomcommanddeploymentstep.cpp",
        "remotelinuxruncontrol.cpp",
        "remotelinuxruncontrolfactory.cpp",
        "remotelinuxruncontrolfactory.h",
        "tarpackagecreationstep.cpp",
        "uploadandinstalltarpackagestep.cpp",
        "remotelinuxcheckforfreediskspaceservice.h",
        "remotelinuxcheckforfreediskspaceservice.cpp",
        "remotelinuxcheckforfreediskspacestep.h",
        "remotelinuxcheckforfreediskspacestep.cpp",
        "remotelinuxcheckforfreediskspacestepwidget.ui",
        "remotelinuxdeploymentdatamodel.h",
        "remotelinuxdeploymentdatamodel.cpp",
        "images/embeddedtarget.png"
    ]

    ProductModule {
        Depends { name: "Core" }
        Depends { name: "QtcSsh" }
    }
}

