/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2012 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef BUILDCONFIGURATIONINFO_H
#define BUILDCONFIGURATIONINFO_H

#include "qt4projectmanager_global.h"
#include <coreplugin/featureprovider.h>
#include <qtsupport/baseqtversion.h>

namespace Qt4ProjectManager {
struct QT4PROJECTMANAGER_EXPORT BuildConfigurationInfo {
    explicit BuildConfigurationInfo()
        : qtVersionId(-1), buildConfig(QtSupport::BaseQtVersion::QmakeBuildConfig(0)), importing(false), temporaryQtVersion(0)
    {}

    explicit BuildConfigurationInfo(int v, QtSupport::BaseQtVersion::QmakeBuildConfigs bc,
                                    const QString &aa, const QString &d,
                                    bool importing_ = false,
                                    QtSupport::BaseQtVersion *temporaryQtVersion_ = 0,
                                    const QString &makefile_ = QString())
        : qtVersionId(v), buildConfig(bc),
          additionalArguments(aa), directory(d),
          importing(importing_), temporaryQtVersion(temporaryQtVersion_),
          makefile(makefile_)
    { }

    bool isValid() const
    {
        return version() != 0;
    }

    bool operator ==(const BuildConfigurationInfo &other) const
    {
        return qtVersionId == other.qtVersionId
                && buildConfig == other.buildConfig
                && additionalArguments == other.additionalArguments
                && directory == other.directory
                && importing == other.importing
                && temporaryQtVersion == other.temporaryQtVersion
                && makefile == other.makefile;
    }
    QtSupport::BaseQtVersion *version() const;

    int qtVersionId;
    QtSupport::BaseQtVersion::QmakeBuildConfigs buildConfig;
    QString additionalArguments;
    QString directory;
    bool importing;
    QtSupport::BaseQtVersion *temporaryQtVersion;
    QString makefile;

    static QList<BuildConfigurationInfo> importBuildConfigurations(const QString &proFilePath);
    static QList<BuildConfigurationInfo> checkForBuild(const QString &directory, const QString &proFilePath);
    static QList<BuildConfigurationInfo> filterBuildConfigurationInfos(const QList<BuildConfigurationInfo> &infos, const QString &id);
    static QList<BuildConfigurationInfo> filterBuildConfigurationInfos(const QList<BuildConfigurationInfo> &infos, Core::FeatureSet features);
    static QList<BuildConfigurationInfo> filterBuildConfigurationInfosByPlatform(const QList<BuildConfigurationInfo> &infos, const QString &platform);
};

} // namespace Qt4ProjectManager

#endif // BUILDCONFIGURATIONINFO_H
