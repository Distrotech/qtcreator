/**************************************************************************
**
** Copyright (C) 2012 - 2014 BlackBerry Limited. All rights reserved.
**
** Contact: BlackBerry (qt@blackberry.com)
** Contact: KDAB (info@kdab.com)
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company.  For licensing terms and
** conditions see http://www.qt.io/terms-conditions.  For further information
** use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file.  Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, The Qt Company gives you certain additional
** rights.  These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#ifndef QNX_QNXCONSTANTS_H
#define QNX_QNXCONSTANTS_H

#include <QtGlobal>

namespace Qnx {

enum QnxArchitecture {
    X86,
    ArmLeV7,
    UnknownArch
};

namespace Constants {

const int QNX_BB_QT_FACTORY_PRIO  = 60;
const int QNX_QNX_QT_FACTORY_PRIO = 50;

const char QNX_TARGET_KEY[] = "QNX_TARGET";
const char QNX_HOST_KEY[]   = "QNX_HOST";

const char QNX_BB_QT[]  = "Qt4ProjectManager.QtVersion.QNX.BlackBerry";
const char QNX_QNX_QT[] = "Qt4ProjectManager.QtVersion.QNX.QNX";

const char QNX_BB_FEATURE[]  = "QtSupport.Wizards.FeatureBlackBerry";
const char QNX_QNX_FEATURE[] = "QtSupport.Wizards.FeatureQNX";

const char QNX_BB_X86_TARGET_ID[]     = "Qt4ProjectManager.Target.QNX.BBX86Target";
const char QNX_BB_ARMLEV7_TARGET_ID[] = "Qt4ProjectManager.Target.QNX.BBArmLeV7Target";
const char QNX_QNX_X86_TARGET_ID[]     = "Qt4ProjectManager.Target.QNX.QNXX86Target";
const char QNX_QNX_ARMLEV7_TARGET_ID[] = "Qt4ProjectManager.Target.QNX.QNXArmLeV7Target";

const char QNX_BB_DEPLOYCONFIGURATION_ID[]     = "Qt4ProjectManager.QNX.BBDeployConfiguration";
const char QNX_QNX_DEPLOYCONFIGURATION_ID[]     = "Qt4ProjectManager.QNX.QNXDeployConfiguration";

const char QNX_BB_RUNCONFIGURATION_PREFIX[] = "Qt4ProjectManager.QNX.BBRunConfiguration.";
const char QNX_QNX_RUNCONFIGURATION_PREFIX[] = "Qt4ProjectManager.QNX.QNXRunConfiguration.";

const char QNX_CREATE_PACKAGE_BS_ID[] = "Qt4ProjectManager.QnxCreatePackageBuildStep";
const char QNX_DEPLOY_PACKAGE_BS_ID[] = "Qt4ProjectManager.QnxDeployPackageBuildStep";
const char QNX_CHECK_DEVICE_STATUS_BS_ID[] = "Qt4ProjectManager.QnxCheckDeviceStatusBuildStep";

const char QNX_PROFILEPATH_KEY[]   = "Qt4ProjectManager.QnxRunConfiguration.ProFilePath";

const char QNX_BB_OS_TYPE[]  = "BBOsType";
const char QNX_QNX_OS_TYPE[] = "QnxOsType";

const char QNX_DEBUG_TOKEN_KEY[] = "debugToken";

const char QNX_BLACKBERRY_CASCADES_WIZARD_ID[]     = "Q.QnxBlackBerryCascadesApp";
const char QNX_BAR_DESCRIPTOR_WIZARD_ID[]          = "Q.QnxBlackBerryBarDescriptor";
const char QNX_BLACKBERRY_QTQUICK_APP_WIZARD_ID[]  = "Q.QnxBlackBerryQQApp";
const char QNX_BLACKBERRY_QTQUICK2_APP_WIZARD_ID[] = "Q.QnxBlackBerryQQ2App";
const char QNX_BLACKBERRY_GUI_APP_WIZARD_ID[]      = "Q.QnxBlackBerryGuiApp";
const char QNX_BLACKBERRY_SETUP_WIZARD_ID[]        = "Q.QnxBlackBerrySetupWizardId";

const char QNX_QNX_PLATFORM_NAME[] = "QNX";
const char QNX_BB_PLATFORM_NAME[]  = "BlackBerry";

const char QNX_DEBUG_EXECUTABLE[] = "pdebug";

const char QNX_TOOLCHAIN_ID[] = "Qnx.QccToolChain";

// BlackBerry settings constants
const char QNX_BB_CATEGORY[] = "XF.BlackBerry";
const char QNX_CATEGORY[] = "XF.Qnx";
const char QNX_BB_CATEGORY_TR[] = QT_TRANSLATE_NOOP("BlackBerry", "BlackBerry");
const char QNX_CATEGORY_TR[] = QT_TRANSLATE_NOOP("QNX", "QNX");
const char QNX_BB_CATEGORY_ICON[] = ":/qnx/images/target.png";
const char QNX_CATEGORY_ICON[] = ":/qnx/images/qnx-target.png";
const char QNX_BB_SETUP_ID[] = "ZA.BlackBerry Setup";
const char QNX_BB_NDK_SETTINGS_ID[] = "ZZ.BlackBerry NDK Configuration";
const char QNX_SETTINGS_ID[] = "ZZ.Qnx Configuration";
const char QNX_BB_SIGNING_ID[] = "ZZ.BlackBerry Signing Infrastructure Configuration";

const char QNX_BAR_DESCRIPTOR_MIME_TYPE[] = "application/vnd.rim.qnx.bar_descriptor";
const char QNX_BAR_DESCRIPTOR_EDITOR_ID[] = "Qnx.BarDescriptorEditor";
const char QNX_BAR_DESCRIPTOR_EDITOR_CONTEXT[] = "Qnx.BarDescriptorEditor";

const char QNX_TASK_CATEGORY_BARDESCRIPTOR[] = "Task.Category.BarDescriptor";

const char QNX_KEY_AUTHOR[] = "author";
const char QNX_KEY_PATH[] = "path";
const char QNX_KEY_ACTIVE[] = "active";

const char QNX_BLACKBERRY_DEPLOY_CMD[] = "blackberry-deploy";

const char QNX_BLACKBERRY_CASCADESIMPORTER_VERSION[] = "0.0.1";

const char QNX_BLACKBERRY_CONFIGS_FILENAME[] = "bbndkconfigurations.xml";
const char QNX_CONFIGS_FILENAME[] = "qnxconfigurations.xml";

const char QNX_DEBUGGING_GROUP[] = "Debugger.Group.Qnx";

const char QNX_BLACKBERRY_DEFAULT_DEPLOY_QT_BASEPATH[] = "/accounts/devuser/";

const char QNX_OK_ICON[] = ":/qnx/images/ok.png";

const char QNX_BLACKBERRY_SETUP_URL[] = "http://wiki.qt.io/Qt_Creator_with_BlackBerry_10";
const char QNX_LEGACY_KEYS_URL[] = "https://developer.blackberry.com/native/documentation/core"
        "/com.qnx.doc.ide.userguide/topic/signing_and_publishing.html";
const char QNX_REGISTER_KEYS_URL[] = "https://www.blackberry.com/SignedKeys/codesigning.html";

const char QNX_BB_KEY_CONFIGURATION_TYPE[] = "BBConfigurationType";

const char QNX_BB_APILEVEL_TYPE[] = "BBApiLevel";
const char QNX_BB_RUNTIME_TYPE[] = "BBRuntime";

} // namespace Constants
} // namespace Qnx

#endif // QNX_QNXCONSTANTS_H
