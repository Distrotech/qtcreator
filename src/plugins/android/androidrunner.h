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

#ifndef ANDROIDRUNNER_H
#define ANDROIDRUNNER_H

#include "androidconfigurations.h"

#include <QObject>
#include <QStringList>
#include <QTimer>
#include <QThread>
#include <QProcess>
#include <QMutex>


namespace Android {
namespace Internal {
class AndroidRunConfiguration;

class AndroidRunner : public QThread
{
    Q_OBJECT

public:
    AndroidRunner(QObject *parent, AndroidRunConfiguration *runConfig,
        bool debugging);
    ~AndroidRunner();

    QString displayName() const;

public slots:
    void start();
    void stop();

signals:
    void remoteProcessStarted(int gdbServerPort = -1, int qmlPort = -1);
    void remoteProcessFinished(const QString &errString = QString());

    void remoteOutput(const QByteArray &output);
    void remoteErrorOutput(const QByteArray &output);

private slots:
    void killPID();
    void checkPID();
    void logcatReadStandardError();
    void logcatReadStandardOutput();
    void startLogcat();
    void asyncStart();
    void asyncStop();

private:
    void adbKill(qint64 pid, const QString &device, int timeout = 2000, const QString &runAsPackageName = QString());

private:
    bool m_debugingMode;
    QProcess m_adbLogcatProcess;
    QByteArray m_logcat;
    QString m_intentName;
    QString m_packageName;
    QString m_deviceSerialNumber;
    qint64 m_processPID;
    qint64 m_gdbserverPID;
    QTimer m_checkPIDTimer;
    QString m_remoteChannel;
    bool m_useLocalQtLibs;
    QString m_localLibs;
    QString m_localJars;
    QMutex m_mutex;
};

} // namespace Internal
} // namespace Android

#endif // ANDROIDRUNNER_H
