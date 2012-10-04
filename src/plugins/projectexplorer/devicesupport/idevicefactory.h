/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2012 Nokia Corporation and/or its subsidiary(-ies).
**
** Contact: http://www.qt-project.org/
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
**
**************************************************************************/
#ifndef IDEVICEFACTORY_H
#define IDEVICEFACTORY_H

#include "idevice.h"
#include <projectexplorer/projectexplorer_export.h>

#include <QObject>
#include <QVariantMap>

QT_BEGIN_NAMESPACE
class QWidget;
QT_END_NAMESPACE

namespace ProjectExplorer {
class IDeviceWidget;

/*!
  \class ProjectExplorer::IDeviceFactory

  \brief Provides an interface for classes providing services related to certain type of device.

  The factory objects have to be added to the global object pool via
  \c ExtensionSystem::PluginManager::addObject().
  \sa ExtensionSystem::PluginManager::addObject()
*/
class PROJECTEXPLORER_EXPORT IDeviceFactory : public QObject
{
    Q_OBJECT

public:
    /*!
      A short, one-line description of what the device type.
    */
    virtual QString displayNameForId(Core::Id type) const = 0;
    /*!
      A list of device types this factory can create.
    */
    virtual QList<Core::Id> availableCreationIds() const = 0;

    /*!
      Check whether this factory can create new devices. This is used to hide
      auto-detect-only factories from the listing of possible devices to create.
     */
    virtual bool canCreate() const;

    /*!
      Create a new device. This may or may not open a wizard.
     */
    virtual IDevice::Ptr create(Core::Id id) const = 0;

    /*!
      Check whether this factory can restore a device from the given serialized state.
     */
    virtual bool canRestore(const QVariantMap &map) const = 0;

    /*!
      Loads a device from a serialized state. Will only ever be called if canRestore()
      returns true for the given map.
    */
    virtual IDevice::Ptr restore(const QVariantMap &map) const = 0;

    static IDeviceFactory *find(Core::Id type);

protected:
    IDeviceFactory(QObject *parent = 0);
};

} // namespace ProjectExplorer

#endif // IDEVICEFACTORY_H
