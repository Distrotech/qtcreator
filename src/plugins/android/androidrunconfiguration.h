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

#ifndef ANDROIDRUNCONFIGURATION_H
#define ANDROIDRUNCONFIGURATION_H

#include "androidconstants.h"
#include "androidconfigurations.h"

#include <utils/environment.h>

#include <projectexplorer/runconfiguration.h>

QT_FORWARD_DECLARE_CLASS(QWidget)

namespace Qt4ProjectManager {

class Qt4BuildConfiguration;
class Qt4Project;
class Qt4ProFileNode;
}

namespace Android {
namespace Internal {

class AndroidDeviceConfigListModel;
class AndroidDeployStep;
class AndroidRunConfigurationFactory;
class AndroidToolChain;
class AndroidTarget;

class AndroidRunConfiguration : public ProjectExplorer::RunConfiguration
{
    Q_OBJECT
    friend class AndroidRunConfigurationFactory;

public:
    enum BaseEnvironmentBase {
        CleanEnvironmentBase = 0,
        SystemEnvironmentBase = 1
    };

    enum DebuggingType { DebugCppOnly, DebugQmlOnly, DebugCppAndQml };

    AndroidRunConfiguration(AndroidTarget *parent, const QString &proFilePath);
    virtual ~AndroidRunConfiguration();

    QWidget *createConfigurationWidget();
    Utils::OutputFormatter *createOutputFormatter() const;
    AndroidTarget *androidTarget() const;
    Qt4ProjectManager::Qt4BuildConfiguration *activeQt4BuildConfiguration() const;

    AndroidDeployStep *deployStep() const;

    void setArguments(const QString &args);
    AndroidConfig config() const;
    QString proFilePath() const;

    DebuggingType debuggingType() const;

    const QString gdbCmd() const;
    const QString remoteChannel() const;
    const QString dumperLib() const;

protected:
    AndroidRunConfiguration(AndroidTarget *parent, AndroidRunConfiguration *source);
    QString defaultDisplayName();

private:
    void init();

    QString m_proFilePath;

};

} // namespace Internal
} // namespace Android

#endif // ANDROIDRUNCONFIGURATION_H
