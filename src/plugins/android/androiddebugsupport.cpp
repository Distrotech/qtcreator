/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2012 BogDan Vatra <bog_dan_ro@yahoo.com>
**
** Contact: Nokia Corporation (qt-info@nokia.com)
**
**
** GNU Lesser General Public License Usage
**
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this file.
** Please review the following information to ensure the GNU Lesser General
** Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** Other Usage
**
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**************************************************************************/

#include "androiddebugsupport.h"

#include "androiddeploystep.h"
#include "androidglobal.h"
#include "androidrunner.h"
#include "androidtarget.h"

#include <debugger/debuggerplugin.h>
#include <debugger/debuggerrunner.h>
#include <debugger/debuggerengine.h>
#include <debugger/debuggerstartparameters.h>

#include <qt4projectmanager/qt4buildconfiguration.h>
#include <qt4projectmanager/qt4target.h>
#include <qt4projectmanager/qt4project.h>
#include <qt4projectmanager/qt4nodes.h>

#include <QDir>

using namespace Debugger;
using namespace ProjectExplorer;
using namespace Qt4ProjectManager;

namespace Android {
namespace Internal {

static const char * const qMakeVariables[] = {
         "QT_INSTALL_LIBS",
         "QT_INSTALL_PLUGINS",
         "QT_INSTALL_IMPORTS"
};


RunControl *AndroidDebugSupport::createDebugRunControl(AndroidRunConfiguration *runConfig)
{
    DebuggerStartParameters params;
    params.toolChainAbi = runConfig->abi();
    params.dumperLibrary = runConfig->dumperLib();
    params.startMode = AttachToRemoteServer;
    params.executable = runConfig->androidTarget()->qt4Project()->rootQt4ProjectNode()->buildDir() + QLatin1String("/app_process");
    params.debuggerCommand = runConfig->gdbCmd();
    params.remoteChannel = runConfig->remoteChannel();
    params.displayName = runConfig->androidTarget()->packageName();

    params.solibSearchPath.clear();

    QList<Qt4ProFileNode *> nodes = runConfig->androidTarget()->qt4Project()->allProFiles();
    foreach (Qt4ProFileNode *node, nodes)
        if (node->projectType() == ApplicationTemplate)
            params.solibSearchPath.append(node->targetInformation().buildDir);

    params.solibSearchPath.append(qtSoPaths(runConfig->activeQt4BuildConfiguration()->qtVersion()));

    params.useServerStartScript = true;
    params.requestRemoteSetup = true;
    params.remoteArchitecture = QLatin1String("arm");

    DebuggerRunControl * const debuggerRunControl
        = DebuggerPlugin::createDebugger(params, runConfig);
    new AndroidDebugSupport(runConfig, debuggerRunControl);
    return debuggerRunControl;
}

AndroidDebugSupport::AndroidDebugSupport(AndroidRunConfiguration *runConfig,
    DebuggerRunControl *runControl)
    : QObject(runControl), m_runControl(runControl),
      m_runner(new AndroidRunner(this, runConfig, true)),
      m_debuggingType(runConfig->debuggingType()),
      m_gdbServerPort(5039), m_qmlPort(-1)
{
    connect(m_runControl->engine(), SIGNAL(requestRemoteSetup()),
            m_runner, SLOT(start()));
    connect(m_runControl, SIGNAL(finished()),
            m_runner, SLOT(stop()));

    connect(m_runner, SIGNAL(remoteProcessStarted(int,int)),
        SLOT(handleRemoteProcessStarted(int,int)));
    connect(m_runner, SIGNAL(remoteProcessFinished(const QString &)),
        SLOT(handleRemoteProcessFinished(const QString &)));

    connect(m_runner, SIGNAL(remoteErrorOutput(QByteArray)),
        SLOT(handleRemoteErrorOutput(QByteArray)));
    connect(m_runner, SIGNAL(remoteOutput(QByteArray)),
        SLOT(handleRemoteOutput(QByteArray)));
}

AndroidDebugSupport::~AndroidDebugSupport()
{
}

void AndroidDebugSupport::handleRemoteProcessStarted(int gdbServerPort, int qmlPort)
{
    disconnect(m_runner, SIGNAL(remoteProcessStarted(int,int)),
        this, SLOT(handleRemoteProcessStarted(int,int)));
    m_runControl->engine()->handleRemoteSetupDone(gdbServerPort, qmlPort);
}

void AndroidDebugSupport::handleRemoteProcessFinished(const QString &errorMsg)
{
    disconnect(m_runner, SIGNAL(remoteProcessFinished(const QString &)),
        this,SLOT(handleRemoteProcessFinished(const QString &)));
    m_runControl->engine()->handleRemoteSetupFailed(errorMsg);
}

void AndroidDebugSupport::handleRemoteOutput(const QByteArray &output)
{
    if (m_runControl)
        m_runControl->showMessage(QString::fromUtf8(output), AppOutput);
}

void AndroidDebugSupport::handleRemoteErrorOutput(const QByteArray &output)
{
   if (m_runControl)
        m_runControl->showMessage(QString::fromUtf8(output), AppError);
}

QStringList AndroidDebugSupport::qtSoPaths(QtSupport::BaseQtVersion *qtVersion)
{
    QSet<QString> paths;
    for (uint i = 0; i < sizeof qMakeVariables / sizeof qMakeVariables[0]; ++i) {
        if (!qtVersion->versionInfo().contains(QLatin1String(qMakeVariables[i])))
            continue;
        QDirIterator it(qtVersion->versionInfo()[QLatin1String(qMakeVariables[i])], QStringList() << QLatin1String("*.so"), QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            it.next();
            paths.insert(it.fileInfo().absolutePath());
        }
    }
    return paths.toList();
}

} // namespace Internal
} // namespace Qt4ProjectManager
