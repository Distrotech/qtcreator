TEMPLATE = lib
TARGET = Qnx
QT += network xml
PROVIDER = BlackBerry

include(../../qtcreatorplugin.pri)
include(qnx_dependencies.pri)

SOURCES += qnxplugin.cpp \
    blackberryqtversionfactory.cpp \
    blackberryqtversion.cpp \
    qnxbaseqtconfigwidget.cpp \
    blackberrydeployconfigurationfactory.cpp \
    blackberrydeployconfiguration.cpp \
    blackberrycreatepackagestep.cpp \
    blackberrycreatepackagestepconfigwidget.cpp \
    blackberrycreatepackagestepfactory.cpp \
    blackberrydeploystep.cpp \
    blackberrydeployconfigurationwidget.cpp \
    blackberrydeploystepconfigwidget.cpp \
    blackberrydeviceconfigurationfactory.cpp \
    blackberrydeviceconfigurationwizard.cpp \
    blackberrydeviceconfigurationwizardpages.cpp \
    blackberrydeploystepfactory.cpp \
    blackberryrunconfiguration.cpp \
    blackberryrunconfigurationwidget.cpp \
    blackberryrunconfigurationfactory.cpp \
    blackberryruncontrolfactory.cpp \
    blackberryruncontrol.cpp \
    blackberrydebugsupport.cpp \
    blackberryapplicationrunner.cpp \
    blackberryconnect.cpp \
    qnxutils.cpp \
    blackberrydeviceconfigurationwidget.cpp \
    qnxdeviceconfigurationfactory.cpp \
    qnxdeviceconfigurationwizard.cpp \
    qnxdeviceconfigurationwizardpages.cpp \
    qnxrunconfiguration.cpp \
    qnxruncontrolfactory.cpp \
    qnxdebugsupport.cpp \
    qnxdeploystepfactory.cpp \
    qnxdeployconfigurationfactory.cpp \
    qnxrunconfigurationfactory.cpp \
    qnxruncontrol.cpp \
    qnxqtversionfactory.cpp \
    qnxqtversion.cpp \
    qnxabstractqtversion.cpp \
    blackberrydeviceconfiguration.cpp \
    qnxdeployconfiguration.cpp \
    qnxdeviceconfiguration.cpp \
    blackberrydeployinformation.cpp \
    pathchooserdelegate.cpp \
    blackberryabstractdeploystep.cpp \
    blackberryndksettingswidget.cpp \
    blackberryndksettingspage.cpp \
    blackberryconfiguration.cpp \
    bardescriptormagicmatcher.cpp \
    bardescriptoreditorfactory.cpp \
    bardescriptoreditor.cpp \
    bardescriptoreditorwidget.cpp \
    bardescriptordocument.cpp \
    bardescriptordocumentnodehandlers.cpp \
    bardescriptorpermissionsmodel.cpp \
    blackberrykeyswidget.cpp \
    blackberrykeyspage.cpp \
    blackberrycsjregistrar.cpp \
    blackberrycertificate.cpp \
    blackberrycertificatemodel.cpp \
    blackberryregisterkeydialog.cpp \
    blackberryimportcertificatedialog.cpp \
    blackberrycreatecertificatedialog.cpp

HEADERS += qnxplugin.h\
    qnxconstants.h \
    blackberryqtversionfactory.h \
    blackberryqtversion.h \
    qnxbaseqtconfigwidget.h \
    blackberrydeployconfigurationfactory.h \
    blackberrydeployconfiguration.h \
    blackberrycreatepackagestep.h \
    blackberrycreatepackagestepconfigwidget.h \
    blackberrycreatepackagestepfactory.h \
    blackberrydeploystep.h \
    blackberrydeployconfigurationwidget.h \
    blackberrydeploystepconfigwidget.h \
    blackberrydeviceconfigurationfactory.h \
    blackberrydeviceconfigurationwizard.h \
    blackberrydeviceconfigurationwizardpages.h \
    blackberrydeploystepfactory.h \
    blackberryrunconfiguration.h \
    blackberryrunconfigurationwidget.h \
    blackberryrunconfigurationfactory.h \
    blackberryruncontrolfactory.h \
    blackberryruncontrol.h \
    blackberrydebugsupport.h \
    blackberryapplicationrunner.h \
    blackberryconnect.h \
    qnxutils.h \
    blackberrydeviceconfigurationwidget.h \
    qnxdeviceconfigurationfactory.h \
    qnxdeviceconfigurationwizard.h \
    qnxdeviceconfigurationwizardpages.h \
    qnxrunconfiguration.h \
    qnxruncontrolfactory.h \
    qnxdebugsupport.h \
    qnxdeploystepfactory.h \
    qnxdeployconfigurationfactory.h \
    qnxrunconfigurationfactory.h \
    qnxruncontrol.h \
    qnxqtversionfactory.h \
    qnxqtversion.h \
    qnxabstractqtversion.h \
    blackberrydeviceconfiguration.h \
    qnxdeployconfiguration.h \
    qnxdeviceconfiguration.h \
    blackberrydeployinformation.h \
    pathchooserdelegate.h \
    blackberryabstractdeploystep.h \
    blackberryndksettingswidget.h \
    blackberryndksettingspage.h \
    blackberryconfiguration.h \
    bardescriptormagicmatcher.h \
    bardescriptoreditorfactory.h \
    bardescriptoreditor.h \
    bardescriptoreditorwidget.h \
    bardescriptordocument.h \
    bardescriptordocumentnodehandlers.h \
    bardescriptorpermissionsmodel.h \
    blackberrykeyswidget.h \
    blackberrykeyspage.h \
    blackberrycsjregistrar.h \
    blackberrycertificate.h \
    blackberrycertificatemodel.h \
    blackberryregisterkeydialog.h \
    blackberryimportcertificatedialog.h \
    blackberrycreatecertificatedialog.h

FORMS += \
    blackberrydeviceconfigurationwizardsetuppage.ui \
    blackberryrunconfigurationwidget.ui \
    blackberrydeviceconfigurationwizardsshkeypage.ui \
    blackberrydeployconfigurationwidget.ui \
    blackberrydeviceconfigurationwidget.ui \
    qnxbaseqtconfigwidget.ui \
    blackberryndksettingswidget.ui \
    bardescriptoreditorwidget.ui \
    blackberrykeyswidget.ui \
    blackberryregisterkeydialog.ui \
    blackberryimportcertificatedialog.ui \
    blackberrycreatecertificatedialog.ui

DEFINES += QT_NO_CAST_TO_ASCII QT_NO_CAST_FROM_ASCII

RESOURCES += \
    qnx.qrc
