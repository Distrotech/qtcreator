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
#include "idevice.h"

#include "devicemanager.h"

#include <coreplugin/id.h>
#include <utils/qtcassert.h>

#include <QCoreApplication>

#include <QString>
#include <QUuid>

/*!
 * \class ProjectExplorer::IDevice
 * \brief This is the base class for all devices.
 *
 * The term "device" refers
 * here to some host to which e.g. files can be deployed or on which an application can run.
 * In the typical case, this would be some sort of embedded computer connected in some way to
 * the PC on which QtCreator runs. This class itself does not specify a connection protocol; that
 * kind of detail is to be added by subclasses.
 * Devices are managed by a \c DeviceManager.
 * \sa ProjectExplorer::DeviceManager
 */

/*!
 * \fn QString ProjectExplorer::IDevice::type() const
 * \brief Identifies the type of the device.
 * Devices with the same type share certain abilities.
 * This attribute is immutable.
 * \sa ProjectExplorer::IDeviceFactory
 */


/*!
 * \fn QString ProjectExplorer::IDevice::displayName() const
 * \brief A free-text name for the device to be displayed in GUI elements.
 */

/*!
 * \fn bool ProjectExplorer::IDevice::isAutoDetected() const
 * \brief True iff the device has been added via some sort of auto-detection mechanism.
 * Devices that are not auto-detected can only ever be created interactively from the
 * settings page.
 * This attribute is immutable.
 * \sa DeviceSettingsWidget
 */

/*!
 * \fn Core::Id ProjectExplorer::IDevice::id() const
 * \brief Identify the device.
 * If an id is given when constructing a device then this id is used. Otherwise a UUID is
 * generated and used to identity the device.
 * \sa ProjectExplorer::DeviceManager::findInactiveAutoDetectedDevice()
 */

/*!
 * \fn Core::Id ProjectExplorer::IDevice::invalidId()
 * \brief A value that no device can ever have as its internal id.
 */

/*!
 * \fn QString ProjectExplorer::IDevice::displayType() const
 * \brief Prints a representation of the device's type suitable for displaying to a user.
 */

/*!
 * \fn ProjectExplorer::IDeviceWidget *ProjectExplorer::IDevice::createWidget() const
 * \brief Creates a widget that displays device information not part of the IDevice base class.
 *        The widget can also be used to let the user change these attributes.
 */

/*!
 * \fn QStringList ProjectExplorer::IDevice::actionIds() const
 * \brief Returns a list of ids representing actions that can be run on this device.
 *        These actions will be available in the "Devices" options page.
 */

/*!
 * \fn QString ProjectExplorer::IDevice::displayNameForActionId(const QString &actionId) const
 * \brief A human-readable string for the given id. Will be displayed on a button which,
 *        when clicked, starts the respective action.
 */

/*!
 * \fn QDialog *ProjectExplorer::IDevice::createAction(const QString &actionId) const
 * \brief Produces a dialog implementing the respective action. The dialog is supposed to be
 *        modal, so implementers must make sure to make whatever it does interruptible as
 *        to not needlessly block the UI.
 */

/*!
 * \fn ProjectExplorer::IDevice::Ptr ProjectExplorer::IDevice::clone() const
 * \brief Creates an identical copy of a device object.
 */

/*!
 * \fn void fromMap(const QVariantMap &map)
 * \brief Restores a device object from a serialized state as written by \c toMap().
 * If subclasses override this to restore additional state, they must call the base class
 * implementation.
 */

/*!
 * \fn QVariantMap toMap() const
 * \brief Serializes a device object, e.g. to save it to a file.
 * If subclasses override this to save additional state, they must call the base class
 * implementation.
 */

static Core::Id newId()
{
    return Core::Id(QUuid::createUuid().toString());
}

namespace ProjectExplorer {

const char DisplayNameKey[] = "Name";
const char TypeKey[] = "OsType";
const char IdKey[] = "InternalId";
const char OriginKey[] = "Origin";

namespace Internal {
class IDevicePrivate
{
public:
    QString displayName;
    QString type;
    IDevice::Origin origin;
    Core::Id id;
    IDevice::AvailabilityState availability;
};
} // namespace Internal

IDevice::IDevice() : d(new Internal::IDevicePrivate)
{
}

IDevice::IDevice(const QString &type, Origin origin, const Core::Id &id)
    : d(new Internal::IDevicePrivate)
{
    d->type = type;
    d->origin = origin;
    QTC_CHECK(origin == ManuallyAdded || id.isValid());
    d->id = id.isValid() ? id : newId();
    d->availability = DeviceAvailabilityUnknown;
}

IDevice::IDevice(const IDevice &other) : d(new Internal::IDevicePrivate)
{
    *d = *other.d;
}

IDevice::~IDevice()
{
    delete d;
}

QString IDevice::displayName() const
{
    return d->displayName;
}

void IDevice::setDisplayName(const QString &name)
{
    if (d->displayName == name)
        return;
    d->displayName = name;
}

IDevice::DeviceInfo IDevice::deviceInformation() const
{
    DeviceInfo result;
    if (availability() == DeviceUnavailable)
        //: Title of the connectivity state information in a tool tip
        result << IDevice::DeviceInfoItem(QCoreApplication::translate("ProjectExplorer::IDevice", "Device"),
                                          //: Device is not connected
                                          QCoreApplication::translate("ProjectExplorer::IDevice", "not connected"));
    else if (availability() == DeviceAvailable)
        //: Title of the connectivity state information in a tool tip
        result << IDevice::DeviceInfoItem(QCoreApplication::translate("ProjectExplorer::IDevice", "Device"),
                                          //: Device is not connected
                                          QCoreApplication::translate("ProjectExplorer::IDevice", "connected"));
    return result;
}

QString IDevice::type() const
{
    return d->type;
}

bool IDevice::isAutoDetected() const
{
    return d->origin == AutoDetected;
}

Core::Id IDevice::id() const
{
    return d->id;
}

IDevice::AvailabilityState IDevice::availability() const
{
    return d->availability;
}

void IDevice::setAvailability(const IDevice::AvailabilityState as)
{
    if (d->availability == as)
        return;
    d->availability = as;
}

Core::Id IDevice::invalidId()
{
    return Core::Id();
}

QString IDevice::typeFromMap(const QVariantMap &map)
{
    return map.value(QLatin1String(TypeKey)).toString();
}

void IDevice::fromMap(const QVariantMap &map)
{
    d->type = typeFromMap(map);
    d->displayName = map.value(QLatin1String(DisplayNameKey)).toString();
    d->id = Core::Id(map.value(QLatin1String(IdKey), newId().name()).toByteArray().constData());
    d->origin = static_cast<Origin>(map.value(QLatin1String(OriginKey), ManuallyAdded).toInt());
}

QVariantMap IDevice::toMap() const
{
    QVariantMap map;
    map.insert(QLatin1String(DisplayNameKey), d->displayName);
    map.insert(QLatin1String(TypeKey), d->type);
    map.insert(QLatin1String(IdKey), d->id.name());
    map.insert(QLatin1String(OriginKey), d->origin);
    return map;
}

IDevice::Ptr IDevice::sharedFromThis()
{
    return DeviceManager::instance()->fromRawPointer(this);
}

IDevice::ConstPtr IDevice::sharedFromThis() const
{
    return DeviceManager::instance()->fromRawPointer(this);
}

} // namespace ProjectExplorer
