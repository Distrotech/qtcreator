/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2012 Nokia Corporation and/or its subsidiary(-ies).
**
** Contact: http://www.qt-project.org/
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

#ifndef LOCALPROCESSLIST_H
#define LOCALPROCESSLIST_H

#include "deviceprocesslist.h"

#include <QString>

QT_BEGIN_NAMESPACE
class QProcess;
QT_END_NAMESPACE

namespace ProjectExplorer {
namespace Internal {

class LocalProcessList : public DeviceProcessList
{
    Q_OBJECT

public:
    LocalProcessList(const IDevice::ConstPtr &device, QObject *parent = 0);
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;

    static QList<DeviceProcess> getLocalProcesses();

private:
    void doUpdate();
    void doKillProcess(const DeviceProcess &process);

private slots:
    void handleUpdate();
    void reportDelayedKillStatus();

private:
    const qint64 m_myPid;
    QString m_error;
};

} // namespace Internal
} // namespace RemoteLinux

#endif // LOCALPROCESSLIST_H
