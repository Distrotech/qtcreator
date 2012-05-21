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

#include "androidpackageinstallationfactory.h"

#include "androidpackageinstallationstep.h"

#include <projectexplorer/buildsteplist.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/target.h>
#include <qt4projectmanager/qt4projectmanagerconstants.h>

#include <QCoreApplication>

using namespace ProjectExplorer;

namespace Android {
namespace Internal {

AndroidPackageInstallationFactory::AndroidPackageInstallationFactory(QObject *parent)
    : IBuildStepFactory(parent)
{
}

QList<Core::Id> AndroidPackageInstallationFactory::availableCreationIds(BuildStepList *parent) const
{
    if (parent->id() == Core::Id(ProjectExplorer::Constants::BUILDSTEPS_DEPLOY)
            && parent->target()->id() == Core::Id(Qt4ProjectManager::Constants::ANDROID_DEVICE_TARGET_ID)
            && !parent->contains(Core::Id(AndroidPackageInstallationStep::Id)))
        return QList<Core::Id>() << Core::Id(AndroidPackageInstallationStep::Id);
    return QList<Core::Id>();
}

QString AndroidPackageInstallationFactory::displayNameForId(const Core::Id id) const
{
    if (id == AndroidPackageInstallationStep::Id)
        return QCoreApplication::translate("Qt4ProjectManager::Internal::AndroidPackageInstallationFactory",
                                           "Deploy to device");
    return QString();
}

bool AndroidPackageInstallationFactory::canCreate(BuildStepList *parent, const Core::Id id) const
{
    return parent->id() == Core::Id(ProjectExplorer::Constants::BUILDSTEPS_DEPLOY)
            && id == Core::Id(AndroidPackageInstallationStep::Id)
            && parent->target()->id() == Core::Id(Qt4ProjectManager::Constants::ANDROID_DEVICE_TARGET_ID)
            && !parent->contains(AndroidPackageInstallationStep::Id);
}

BuildStep *AndroidPackageInstallationFactory::create(BuildStepList *parent, const Core::Id id)
{
    Q_ASSERT(canCreate(parent, id));
    return new AndroidPackageInstallationStep(parent);
}

bool AndroidPackageInstallationFactory::canRestore(BuildStepList *parent, const QVariantMap &map) const
{
    return canCreate(parent, idFromMap(map));
}

BuildStep *AndroidPackageInstallationFactory::restore(BuildStepList *parent, const QVariantMap &map)
{
    Q_ASSERT(canRestore(parent, map));
    AndroidPackageInstallationStep * const step = new AndroidPackageInstallationStep(parent);
    if (!step->fromMap(map)) {
        delete step;
        return 0;
    }
    return step;
}

bool AndroidPackageInstallationFactory::canClone(BuildStepList *parent, BuildStep *product) const
{
    return canCreate(parent, product->id());
}

BuildStep *AndroidPackageInstallationFactory::clone(BuildStepList *parent, BuildStep *product)
{
    Q_ASSERT(canClone(parent, product));
    return new AndroidPackageInstallationStep(parent, static_cast<AndroidPackageInstallationStep*>(product));
}

} // namespace Internal
} // namespace Android
