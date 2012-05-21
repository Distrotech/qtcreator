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
#include "devicemanager.h"

#include "idevicefactory.h"

#include <coreplugin/icore.h>
#include <coreplugin/id.h>
#include <extensionsystem/pluginmanager.h>
#include <utils/persistentsettings.h>
#include <utils/qtcassert.h>

#include <QFileInfo>
#include <QHash>
#include <QList>
#include <QMainWindow>
#include <QSettings>
#include <QString>
#include <QVariantHash>
#include <QVariantList>

#include <limits>

namespace ProjectExplorer {
namespace Internal {

static IDevice::Ptr findAutoDetectedDevice(const QList<IDevice::Ptr> &deviceList,
        Core::Id type, const Core::Id id)
{
    foreach (const IDevice::Ptr &device, deviceList) {
        if (device->isAutoDetected() && device->type() == type && device->id() == id)
            return device;
    }
    return IDevice::Ptr();
}

const char DeviceManagerKey[] = "DeviceManager";
const char DeviceListKey[] = "DeviceList";
const char DefaultDevicesKey[] = "DefaultDevices";

class DeviceManagerPrivate
{
public:
    static DeviceManager *clonedInstance;
    QList<IDevice::Ptr> devices;
    QList<IDevice::Ptr> inactiveAutoDetectedDevices;
    QHash<Core::Id, Core::Id> defaultDevices;
};
DeviceManager *DeviceManagerPrivate::clonedInstance = 0;

} // namespace Internal

using namespace Internal;


DeviceManager *DeviceManager::instance(const QString &magicTestToken)
{
    static DeviceManager deviceManager(magicTestToken != QLatin1String("magicTestToken"));
    return &deviceManager;
}

int DeviceManager::deviceCount() const
{
    return d->devices.count();
}

void DeviceManager::replaceInstance()
{
    copy(DeviceManagerPrivate::clonedInstance, instance(), false);
    emit instance()->deviceListChanged();
    emit instance()->updated();
}

void DeviceManager::removeClonedInstance()
{
    delete DeviceManagerPrivate::clonedInstance;
    DeviceManagerPrivate::clonedInstance = 0;
}

DeviceManager *DeviceManager::cloneInstance()
{
    QTC_ASSERT(!DeviceManagerPrivate::clonedInstance, return 0);

    DeviceManagerPrivate::clonedInstance = new DeviceManager;
    copy(instance(), DeviceManagerPrivate::clonedInstance, true);
    return DeviceManagerPrivate::clonedInstance;
}

void DeviceManager::copy(const DeviceManager *source, DeviceManager *target, bool deep)
{
    if (deep) {
        foreach (const IDevice::ConstPtr &device, source->d->devices)
            target->d->devices << device->clone();
    } else {
        target->d->devices = source->d->devices;
    }
    target->d->defaultDevices = source->d->defaultDevices;
}

void DeviceManager::save()
{
    Utils::PersistentSettingsWriter writer;
    writer.saveValue(QLatin1String(DeviceManagerKey), toMap());
    writer.save(settingsFilePath(), QLatin1String("QtCreatorDevices"), Core::ICore::mainWindow());
}

void DeviceManager::load()
{
    Utils::PersistentSettingsReader reader;
    if (reader.load(settingsFilePath()))
        fromMap(reader.restoreValues().value(QLatin1String(DeviceManagerKey)).toMap());
    else
        loadPre2_6();
    ensureOneDefaultDevicePerType();
}

// TODO: Remove in 2.8
void DeviceManager::loadPre2_6()
{
    QSettings *settings = Core::ICore::settings();
    settings->beginGroup(QLatin1String("MaemoDeviceConfigs"));
    const QVariantHash defaultDevsHash = settings->value(QLatin1String("DefaultConfigs")).toHash();
    for (QVariantHash::ConstIterator it = defaultDevsHash.constBegin();
            it != defaultDevsHash.constEnd(); ++it) {
        d->defaultDevices.insert(Core::Id(it.key()), Core::Id(it.value().toString()));
    }
    int count = settings->beginReadArray(QLatin1String("ConfigList"));
    for (int i = 0; i < count; ++i) {
        settings->setArrayIndex(i);
        QVariantMap map;
        foreach (const QString &key, settings->childKeys())
            map.insert(key, settings->value(key));
        const IDeviceFactory * const factory = restoreFactory(map);
        if (!factory)
            continue;
        IDevice::Ptr device = factory->restore(map);
        QTC_ASSERT(device, continue);
        d->devices << device;
    }
    settings->endArray();
    settings->endGroup();
}

void DeviceManager::fromMap(const QVariantMap &map)
{
    const QVariantMap defaultDevsMap = map.value(QLatin1String(DefaultDevicesKey)).toMap();
    for (QVariantMap::ConstIterator it = defaultDevsMap.constBegin();
         it != defaultDevsMap.constEnd(); ++it) {
        d->defaultDevices.insert(Core::Id(it.key()), Core::Id(it.value().toString()));
    }
    const QVariantList deviceList = map.value(QLatin1String(DeviceListKey)).toList();
    foreach (const QVariant &v, deviceList) {
        const QVariantMap map = v.toMap();
        const IDeviceFactory * const factory = restoreFactory(map);
        if (!factory)
            continue;
        const IDevice::Ptr device = factory->restore(map);
        QTC_ASSERT(device, continue);
        if (device->isAutoDetected())
            d->inactiveAutoDetectedDevices << device;
        else
            d->devices << device;
    }
}

QVariantMap DeviceManager::toMap() const
{
    QVariantMap map;
    QVariantMap defaultDeviceMap;
    typedef QHash<Core::Id, Core::Id> TypeIdHash;
    for (TypeIdHash::ConstIterator it = d->defaultDevices.constBegin();
             it != d->defaultDevices.constEnd(); ++it) {
        defaultDeviceMap.insert(it.key().toString(), it.value().toString());
    }
    map.insert(QLatin1String(DefaultDevicesKey), defaultDeviceMap);
    QVariantList deviceList;
    foreach (const IDevice::ConstPtr &device, d->devices)
        deviceList << device->toMap();
    foreach (const IDevice::ConstPtr &device, d->inactiveAutoDetectedDevices)
        deviceList << device->toMap();
    map.insert(QLatin1String(DeviceListKey), deviceList);
    return map;
}

QString DeviceManager::settingsFilePath()
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    return QFileInfo(pm->settings()->fileName()).absolutePath() + QLatin1String("/devices.xml");
}

void DeviceManager::addDevice(const IDevice::Ptr &_device)
{
    const IDevice::Ptr device = _device->clone();
    QTC_ASSERT(this != instance() || device->isAutoDetected(), return);

    QString name = device->displayName();
    const int pos = indexForId(device->id());
    if (pos >= 0) {
        device->setDisplayName(QString()); // For name uniquification to work.
        d->devices[pos] = device;
    }

    // Ensure uniqueness of name.
    if (hasDevice(name)) {
        const QString nameTemplate = name + QLatin1String(" (%1)");
        int suffix = 2;
        do
            name = nameTemplate.arg(QString::number(suffix++));
        while (hasDevice(name));
    }
    device->setDisplayName(name);

    if (!defaultDevice(device->type()))
        d->defaultDevices.insert(device->type(), device->id());
    if (this == instance() && d->clonedInstance)
        d->clonedInstance->addDevice(device->clone());

    if (pos >= 0) {
        emit deviceUpdated(device->id());
    } else {
        d->devices << device;
        if (this == instance()) {
            QList<IDevice::Ptr>::Iterator it = d->inactiveAutoDetectedDevices.begin();
            while (it != d->inactiveAutoDetectedDevices.end()) {
                if (it->data()->id() == device->id()) {
                    d->inactiveAutoDetectedDevices.erase(it);
                    break;
                }
                ++it;
            }
        }
        emit deviceAdded(device->id());
    }

    emit updated();
}

void DeviceManager::removeDevice(Core::Id id)
{
    const IDevice::Ptr device = mutableDevice(id);
    QTC_ASSERT(device, return);
    QTC_ASSERT(this != instance() || device->isAutoDetected(), return);

    const bool wasDefault = d->defaultDevices.value(device->type()) == device->id();
    const Core::Id deviceType = device->type();
    d->devices.removeAt(indexForId(id));
    emit deviceRemoved(device->id());

    if (wasDefault) {
        for (int i = 0; i < d->devices.count(); ++i) {
            if (deviceAt(i)->type() == deviceType) {
                d->defaultDevices.insert(deviceAt(i)->type(), deviceAt(i)->id());
                emit deviceUpdated(deviceAt(i)->id());
                break;
            }
        }
    }
    if (this == instance() && d->clonedInstance)
        d->clonedInstance->removeDevice(id);
    if (this == instance() && device->isAutoDetected())
        d->inactiveAutoDetectedDevices << device;

    emit updated();
}

void DeviceManager::setDefaultDevice(int idx)
{
    QTC_ASSERT(this != instance(), return);
    QTC_ASSERT(idx >= 0 && idx < deviceCount(), return);

    const IDevice::ConstPtr &device = d->devices.at(idx);
    const IDevice::ConstPtr &oldDefaultDevice = defaultDevice(device->type());
    if (device == oldDefaultDevice)
        return;
    d->defaultDevices.insert(device->type(), device->id());
    emit deviceUpdated(device->id());
    emit deviceUpdated(oldDefaultDevice->id());

    emit updated();
}

const IDeviceFactory *DeviceManager::restoreFactory(const QVariantMap &map)
{
    const QList<IDeviceFactory *> &factories
        = ExtensionSystem::PluginManager::instance()->getObjects<IDeviceFactory>();
    foreach (const IDeviceFactory * const factory, factories) {
        if (factory->canRestore(map))
            return factory;
    }
    qWarning("Warning: No factory found for device of type '%s'.",
        qPrintable(IDevice::typeFromMap(map).toString()));
    return 0;
}

DeviceManager::DeviceManager(bool isInstance) : d(new DeviceManagerPrivate)
{
    if (isInstance) {
        load();
        connect(Core::ICore::instance(), SIGNAL(saveSettingsRequested()), SLOT(save()));
    }
}

DeviceManager::~DeviceManager()
{
    delete d;
}

IDevice::ConstPtr DeviceManager::deviceAt(int idx) const
{
    QTC_ASSERT(idx >= 0 && idx < deviceCount(), return IDevice::ConstPtr());
    return d->devices.at(idx);
}

IDevice::Ptr DeviceManager::mutableDevice(Core::Id id) const
{
    const int index = indexForId(id);
    return index == -1 ? IDevice::Ptr() : d->devices.at(index);
}

bool DeviceManager::hasDevice(const QString &name) const
{
    foreach (const IDevice::Ptr &device, d->devices) {
        if (device->displayName() == name)
            return true;
    }
    return false;
}

IDevice::ConstPtr DeviceManager::find(Core::Id id) const
{
    const int index = indexForId(id);
    return index == -1 ? IDevice::ConstPtr() : deviceAt(index);
}

IDevice::ConstPtr DeviceManager::findInactiveAutoDetectedDevice(Core::Id type, Core::Id id)
{
    return findAutoDetectedDevice(d->inactiveAutoDetectedDevices, type, id);
}

IDevice::ConstPtr DeviceManager::defaultDevice(Core::Id deviceType) const
{
    const Core::Id id = d->defaultDevices.value(deviceType, IDevice::invalidId());
    if (id == IDevice::invalidId())
        return IDevice::ConstPtr();
    return find(id);
}

int DeviceManager::indexForId(Core::Id id) const
{
    for (int i = 0; i < d->devices.count(); ++i) {
        if (deviceAt(i)->id() == id)
            return i;
    }
    return -1;
}

Core::Id DeviceManager::deviceId(const IDevice::ConstPtr &device) const
{
    return device ? device->id() : IDevice::invalidId();
}

int DeviceManager::indexOf(const IDevice::ConstPtr &device) const
{
    return indexForId(device->id());
}

void DeviceManager::ensureOneDefaultDevicePerType()
{
    foreach (const IDevice::Ptr &device, d->devices) {
        if (!defaultDevice(device->type()))
            d->defaultDevices.insert(device->type(), device->id());
    }
}

IDevice::Ptr DeviceManager::fromRawPointer(IDevice *device) const
{
    foreach (const IDevice::Ptr &devPtr, d->devices) {
        if (devPtr == device)
            return devPtr;
    }

    if (this == instance() && d->clonedInstance)
        return d->clonedInstance->fromRawPointer(device);

    qWarning("%s: Device not found.", Q_FUNC_INFO);
    return IDevice::Ptr();
}

IDevice::ConstPtr DeviceManager::fromRawPointer(const IDevice *device) const
{
    // The const_cast is safe, because we convert the Ptr back to a ConstPtr before returning it.
    return fromRawPointer(const_cast<IDevice *>(device));
}


} // namespace ProjectExplorer
