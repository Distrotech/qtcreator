QTC_PLUGIN_NAME = Android
QTC_PLUGIN_DEPENDS += \
    coreplugin \
    debugger \
    projectexplorer \
    qt4projectmanager \
    qtsupport \
    texteditor \
    analyzerbase

QTC_LIB_DEPENDS += \
    utils

exists(../../shared/qbs/qbs.pro): \
    QTC_PLUGIN_DEPENDS += \
        qbsprojectmanager
