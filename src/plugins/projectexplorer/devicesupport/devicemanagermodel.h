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
#ifndef DEVICEMANAGERMODEL_H
#define DEVICEMANAGERMODEL_H

#include "../projectexplorer_export.h"
#include "idevice.h"

#include <coreplugin/id.h>

#include <QAbstractListModel>

namespace ProjectExplorer {
namespace Internal { class DeviceManagerModelPrivate; }
class IDevice;
class DeviceManager;

class PROJECTEXPLORER_EXPORT DeviceManagerModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit DeviceManagerModel(const DeviceManager *deviceManager, QObject *parent = 0);
    ~DeviceManagerModel();

    void updateDevice(Core::Id id);

    IDevice::ConstPtr device(int pos) const;
    Core::Id deviceId(int pos) const;
    int indexOf(IDevice::ConstPtr dev) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

private slots:
    void handleDeviceAdded(Core::Id id);
    void handleDeviceRemoved(Core::Id id);
    void handleDeviceUpdated(Core::Id id);
    void handleDeviceListChanged();

private:
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    int indexForId(Core::Id id) const;

    Internal::DeviceManagerModelPrivate * const d;
};

} // namespace ProjectExplorer

#endif // DEVICEMANAGERMODEL_H
