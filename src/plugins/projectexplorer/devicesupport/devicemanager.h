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
#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include "idevice.h"

#include <projectexplorer/projectexplorer_export.h>

#include <QObject>

namespace ProjectExplorer {
class IDevice;
class IDeviceFactory;

namespace Internal {
class DeviceManagerPrivate;
class DeviceSettingsWidget;
} // namespace Internal

class PROJECTEXPLORER_EXPORT DeviceManager : public QObject
{
    Q_OBJECT
    friend class Internal::DeviceSettingsWidget;
    friend class IDevice;

public:
    ~DeviceManager();

    static DeviceManager *instance();

    int deviceCount() const;
    IDevice::ConstPtr deviceAt(int index) const;
    IDevice::ConstPtr find(const Core::Id &id) const;
    IDevice::ConstPtr findInactiveAutoDetectedDevice(const QString &type, const Core::Id id);
    IDevice::ConstPtr defaultDevice(const QString &deviceType) const;
    bool hasDevice(const QString &name) const;
    Core::Id deviceId(const IDevice::ConstPtr &device) const;

    int indexOf(const IDevice::ConstPtr &device) const;

    void addDevice(const IDevice::Ptr &device);
    void removeDevice(int index);

signals:
    void deviceUpdated(const Core::Id &id);

    void deviceAdded(const QSharedPointer<const IDevice> &device);
    void deviceRemoved(int index);
    void displayNameChanged(int index);
    void defaultStatusChanged(int index);
    void deviceListChanged();
    void updated(); // Emitted for all of the above.

private:
    DeviceManager(bool doLoad = false);

    void load();
    void save();
    void loadPre2_6();
    static const IDeviceFactory *restoreFactory(const QVariantMap &map);
    void fromMap(const QVariantMap &map);
    QVariantMap toMap() const;
    void ensureOneDefaultDevicePerType();
    int indexForId(const Core::Id &id) const;

    // For SettingsWidget.
    IDevice::Ptr mutableDeviceAt(int index) const;
    void setDeviceDisplayName(int i, const QString &name);
    void setDefaultDevice(int index);
    static DeviceManager *cloneInstance();
    static void replaceInstance();
    static void removeClonedInstance();

    // For IDevice.
    IDevice::Ptr fromRawPointer(IDevice *device) const;
    IDevice::ConstPtr fromRawPointer(const IDevice *device) const;

    static QString settingsFilePath();
    static void copy(const DeviceManager *source, DeviceManager *target, bool deep);

    Internal::DeviceManagerPrivate * const d;
};

} // namespace ProjectExplorer

#endif // DEVICEMANAGER_H
