TEMPLATE = lib
TARGET = RemoteLinux
QT += network

include(../../qtcreatorplugin.pri)
include(remotelinux_dependencies.pri)

HEADERS += \
    genericembeddedlinuxtarget.h \
    embeddedlinuxtargetfactory.h \
    embeddedlinuxqtversion.h \
    embeddedlinuxqtversionfactory.h \
    remotelinuxplugin.h \
    remotelinux_export.h \
    linuxdeviceconfiguration.h \
    remotelinuxrunconfiguration.h \
    publickeydeploymentdialog.h \
    genericlinuxdeviceconfigurationwizard.h \
    genericlinuxdeviceconfigurationfactory.h \
    remotelinuxrunconfigurationwidget.h \
    remotelinuxrunconfigurationfactory.h \
    remotelinuxapplicationrunner.h \
    remotelinuxruncontrol.h \
    remotelinuxruncontrolfactory.h \
    remotelinuxdebugsupport.h \
    genericlinuxdeviceconfigurationwizardpages.h \
    deployablefile.h \
    deployablefilesperprofile.h \
    deploymentinfo.h \
    abstractremotelinuxdeploystep.h \
    genericdirectuploadstep.h \
    uploadandinstalltarpackagestep.h \
    abstractremotelinuxdeployservice.h \
    abstractuploadandinstallpackageservice.h \
    genericdirectuploadservice.h \
    remotelinuxdeployconfiguration.h \
    remotelinuxdeployconfigurationfactory.h \
    genericremotelinuxdeploystepfactory.h \
    abstractpackagingstep.h \
    tarpackagecreationstep.h \
    remotelinuxpackageinstaller.h \
    packageuploader.h \
    linuxdevicetester.h \
    remotelinux_constants.h \
    linuxdevicetestdialog.h \
    remotelinuxprocesslist.h \
    remotelinuxprocessesdialog.h \
    remotelinuxenvironmentreader.h \
    sshkeydeployer.h \
    typespecificdeviceconfigurationlistmodel.h \
    remotelinuxusedportsgatherer.h \
    remotelinuxutils.h \
    deploymentsettingsassistant.h \
    remotelinuxdeployconfigurationwidget.h \
    profilesupdatedialog.h \
    startgdbserverdialog.h \
    remotelinuxcustomcommanddeployservice.h \
    remotelinuxcustomcommanddeploymentstep.h \
    abstractembeddedlinuxtarget.h \
    genericlinuxdeviceconfigurationwidget.h

SOURCES += \
    genericembeddedlinuxtarget.cpp \
    embeddedlinuxtargetfactory.cpp \
    embeddedlinuxqtversion.cpp \
    embeddedlinuxqtversionfactory.cpp \
    remotelinuxplugin.cpp \
    linuxdeviceconfiguration.cpp \
    remotelinuxrunconfiguration.cpp \
    publickeydeploymentdialog.cpp \
    genericlinuxdeviceconfigurationwizard.cpp \
    genericlinuxdeviceconfigurationfactory.cpp \
    remotelinuxrunconfigurationwidget.cpp \
    remotelinuxrunconfigurationfactory.cpp \
    remotelinuxapplicationrunner.cpp \
    remotelinuxruncontrol.cpp \
    remotelinuxruncontrolfactory.cpp \
    remotelinuxdebugsupport.cpp \
    genericlinuxdeviceconfigurationwizardpages.cpp \
    deployablefilesperprofile.cpp \
    deploymentinfo.cpp \
    abstractremotelinuxdeploystep.cpp \
    genericdirectuploadstep.cpp \
    uploadandinstalltarpackagestep.cpp \
    abstractremotelinuxdeployservice.cpp \
    abstractuploadandinstallpackageservice.cpp \
    genericdirectuploadservice.cpp \
    remotelinuxdeployconfiguration.cpp \
    remotelinuxdeployconfigurationfactory.cpp \
    genericremotelinuxdeploystepfactory.cpp \
    abstractpackagingstep.cpp \
    tarpackagecreationstep.cpp \
    remotelinuxpackageinstaller.cpp \
    packageuploader.cpp \
    linuxdevicetester.cpp \
    linuxdevicetestdialog.cpp \
    remotelinuxprocesslist.cpp \
    remotelinuxprocessesdialog.cpp \
    remotelinuxenvironmentreader.cpp \
    sshkeydeployer.cpp \
    typespecificdeviceconfigurationlistmodel.cpp \
    remotelinuxusedportsgatherer.cpp \
    remotelinuxutils.cpp \
    deploymentsettingsassistant.cpp \
    remotelinuxdeployconfigurationwidget.cpp \
    profilesupdatedialog.cpp \
    startgdbserverdialog.cpp \
    remotelinuxcustomcommanddeployservice.cpp \
    remotelinuxcustomcommanddeploymentstep.cpp \
    abstractembeddedlinuxtarget.cpp \
    genericlinuxdeviceconfigurationwidget.cpp

FORMS += \
    genericlinuxdeviceconfigurationwizardsetuppage.ui \
    linuxdevicetestdialog.ui \
    remotelinuxprocessesdialog.ui \
    remotelinuxdeployconfigurationwidget.ui \
    profilesupdatedialog.ui \
    genericlinuxdeviceconfigurationwidget.ui

RESOURCES += remotelinux.qrc

DEFINES += QT_NO_CAST_TO_ASCII
DEFINES += REMOTELINUX_LIBRARY
