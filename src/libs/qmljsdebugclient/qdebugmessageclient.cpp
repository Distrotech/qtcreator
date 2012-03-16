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

#include "qdebugmessageclient.h"

namespace QmlJsDebugClient {

QDebugMessageClient::QDebugMessageClient(QDeclarativeDebugConnection *client)
    : QDeclarativeDebugClient(QLatin1String("DebugMessages"), client)
{
}

QDebugMessageClient::~QDebugMessageClient()
{
}

void QDebugMessageClient::statusChanged(Status status)
{
    emit newStatus(status);
}

void QDebugMessageClient::messageReceived(const QByteArray &data)
{
    QDataStream ds(data);
    QByteArray command;
    ds >> command;

    if (command == "MESSAGE") {
        if (serviceVersion() == 1.0) {
            QByteArray messagePacket;
            ds >> messagePacket;

            QByteArray debugMessage;
            int type;
            QDataStream ms(messagePacket);
            ms >> type >> debugMessage;
            QDebugContextInfo info;
            emit message(QtMsgType(type), QString::fromUtf8(debugMessage.data()),
                         info);
        } else {
            int type;
            int line;
            QByteArray debugMessage;
            QByteArray file;
            QByteArray function;
            ds >> type >> debugMessage >> file >> line >> function;
            QDebugContextInfo info;
            info.line = line;
            info.file = QString::fromUtf8(file);
            info.function = QString::fromUtf8(function);
            emit message(QtMsgType(type), QString::fromUtf8(debugMessage), info);
        }
    }
}

} // namespace QmlJsDebugClient
