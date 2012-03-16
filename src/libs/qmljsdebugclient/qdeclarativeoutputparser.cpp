/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2012 Nokia Corporation and/or its subsidiary(-ies).
**
** Contact: Nokia Corporation (qt-info@nokia.com)
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

#include "qdeclarativeoutputparser.h"
#include "qmljsdebugclientconstants.h"
#include <QRegExp>

namespace QmlJsDebugClient {

QDeclarativeOutputParser::QDeclarativeOutputParser(QObject *parent)
    : QObject(parent)
{
}

void QDeclarativeOutputParser::setNoOutputText(const QString &text)
{
    m_noOutputText = text;
}

void QDeclarativeOutputParser::processOutput(const QString &output)
{
    m_buffer.append(output);

    while (true) {
        const int nlIndex = m_buffer.indexOf(QLatin1Char('\n'));
        if (nlIndex < 0) // no further complete lines
            break;

        const QString msg = m_buffer.left(nlIndex);
        m_buffer = m_buffer.right(m_buffer.size() - nlIndex - 1);

        // used in Qt4
        static const QString qddserver4 = QLatin1String("QDeclarativeDebugServer: ");
        // used in Qt5
        static const QString qddserver5 = QLatin1String("QML Debugger: ");

        QString status;
        int index = msg.indexOf(qddserver4);
        if (index != -1) {
            status = msg;
            status.remove(0, index + qddserver4.length()); // chop of 'QDeclarativeDebugServer: '
        } else {
            index = msg.indexOf(qddserver5);
            if (index != -1) {
                status = msg;
                status.remove(0, index + qddserver5.length()); // chop of 'QML Debugger: '
            }
        }
        if (!status.isEmpty()) {
            static QString waitingForConnection = QLatin1String(Constants::STR_WAITING_FOR_CONNECTION);
            static QString unableToListen = QLatin1String(Constants::STR_UNABLE_TO_LISTEN);
            static QString debuggingNotEnabled = QLatin1String(Constants::STR_IGNORING_DEBUGGER);
            static QString debuggingNotEnabled2 = QLatin1String(Constants::STR_IGNORING_DEBUGGER2);
            static QString connectionEstablished = QLatin1String(Constants::STR_CONNECTION_ESTABLISHED);

            if (status.startsWith(waitingForConnection)) {
                status.remove(0, waitingForConnection.size()); // chop of 'Waiting for connection '

                static QRegExp waitingTcp(
                            QString::fromLatin1(Constants::STR_ON_PORT_PATTERN));
                if (waitingTcp.indexIn(status) > -1) {
                    bool canConvert;
                    quint16 port = waitingTcp.cap(1).toUShort(&canConvert);
                    if (canConvert)
                        emit waitingForConnectionOnPort(port);
                    continue;
                }

                static QString waitingOst
                        = QLatin1String(Constants::STR_VIA_OST);
                if (status.startsWith(waitingOst))
                    emit waitingForConnectionViaOst();
            } else if (status.startsWith(unableToListen)) {
                //: Error message shown after 'Could not connect ... debugger:"
                emit errorMessage(tr("The port seems to be in use."));
            } else if (status.startsWith(debuggingNotEnabled) || status.startsWith(debuggingNotEnabled2)) {
                //: Error message shown after 'Could not connect ... debugger:"
                emit errorMessage(tr("The application is not set up for QML/JS debugging."));
            } else if (status.startsWith(connectionEstablished)) {
                emit connectionEstablishedMessage();
            } else {
                emit unknownMessage(status);
            }
        } else if (msg.contains(m_noOutputText)) {
            emit noOutputMessage();
        }


    }
}

} // namespace QmLJsDebugClient
