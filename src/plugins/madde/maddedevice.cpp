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
#include "maddedevice.h"

#include "maddedevicetester.h"
#include "maemoconstants.h"

#include <remotelinux/linuxdevicetestdialog.h>
#include <remotelinux/publickeydeploymentdialog.h>
#include <remotelinux/remotelinuxprocessesdialog.h>
#include <remotelinux/remotelinuxprocesslist.h>
#include <remotelinux/remotelinux_constants.h>
#include <utils/qtcassert.h>

using namespace RemoteLinux;

namespace Madde {
namespace Internal {
const char MaddeDeviceTestActionId[] = "Madde.DeviceTestAction";
const char MaddeRemoteProcessesActionId[] = "Madde.RemoteProcessesAction";

MaddeDevice::Ptr MaddeDevice::create()
{
    return Ptr(new MaddeDevice);
}

MaddeDevice::Ptr MaddeDevice::create(const QString &name, Core::Id type,
        MachineType machineType, Origin origin, Core::Id id)
{
    return Ptr(new MaddeDevice(name, type, machineType, origin, id));
}

MaddeDevice::MaddeDevice()
{
}

MaddeDevice::MaddeDevice(const QString &name, Core::Id type, MachineType machineType,
        Origin origin, Core::Id id)
    : LinuxDeviceConfiguration(name, type, machineType, origin, id)
{
}

MaddeDevice::MaddeDevice(const MaddeDevice &other) : LinuxDeviceConfiguration(other)
{
}

ProjectExplorer::IDevice::Ptr MaddeDevice::clone() const
{
    return Ptr(new MaddeDevice(*this));
}

QString MaddeDevice::displayType() const
{
    return maddeDisplayType(type());
}

QList<Core::Id> MaddeDevice::actionIds() const
{
    return QList<Core::Id>() << Core::Id(MaddeDeviceTestActionId)
        << Core::Id(Constants::GenericDeployKeyToDeviceActionId)
        << Core::Id(MaddeRemoteProcessesActionId);
}

QString MaddeDevice::displayNameForActionId(Core::Id actionId) const
{
    QTC_ASSERT(actionIds().contains(actionId), return QString());

    if (actionId == Core::Id(MaddeDeviceTestActionId))
        return tr("Test");
    if (actionId == Core::Id(MaddeRemoteProcessesActionId))
        return tr("Remote Processes...");
    if (actionId == Core::Id(Constants::GenericDeployKeyToDeviceActionId))
        return tr("Deploy Public Key...");
    return QString(); // Can't happen.
}

void MaddeDevice::executeAction(Core::Id actionId, QWidget *parent) const
{
    QTC_ASSERT(actionIds().contains(actionId), return);

    QDialog *d;
    const LinuxDeviceConfiguration::ConstPtr device
        = sharedFromThis().staticCast<const LinuxDeviceConfiguration>();
    if (actionId == Core::Id(MaddeDeviceTestActionId))
        d = new LinuxDeviceTestDialog(device, new MaddeDeviceTester, parent);
    else if (actionId == Core::Id(MaddeRemoteProcessesActionId))
        d = new RemoteLinuxProcessesDialog(new GenericRemoteLinuxProcessList(device), parent);
    else if (actionId == Core::Id(Constants::GenericDeployKeyToDeviceActionId))
        d = PublicKeyDeploymentDialog::createDialog(device, parent);
    if (d)
        d->exec();
}

QString MaddeDevice::maddeDisplayType(Core::Id type)
{
    if (type == Core::Id(Maemo5OsType))
        return tr("Maemo5/Fremantle");
    if (type == Core::Id(HarmattanOsType))
        return tr("MeeGo 1.2 Harmattan");
    return tr("Other MeeGo OS");
}

} // namespace Internal
} // namespace Madde
