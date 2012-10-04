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

#include "backtracecollector.h"

#include <QDebug>
#include <QScopedPointer>
#include <QTemporaryFile>

const char GdbBatchCommands[] =
        "set height 0\n"
        "set width 0\n"
        "thread\n"
        "thread apply all backtrace full\n";

class BacktraceCollectorPrivate
{
public:
    BacktraceCollectorPrivate() : errorOccurred(false) {}

    bool errorOccurred;
    QScopedPointer<QTemporaryFile> commandFile;
    QProcess debugger;
    QString output;
};

BacktraceCollector::BacktraceCollector(QObject *parent) :
    QObject(parent), d(new BacktraceCollectorPrivate)
{
    connect(&d->debugger, SIGNAL(finished(int,QProcess::ExitStatus)),
            SLOT(onDebuggerFinished(int,QProcess::ExitStatus)));
    connect(&d->debugger, SIGNAL(error(QProcess::ProcessError)),
            SLOT(onDebuggerError(QProcess::ProcessError)));
    connect(&d->debugger, SIGNAL(readyRead()), SLOT(onDebuggerOutputAvailable()));
    d->debugger.setProcessChannelMode(QProcess::MergedChannels);
}

BacktraceCollector::~BacktraceCollector()
{
    delete d;
}

void BacktraceCollector::run(Q_PID pid)
{
    d->debugger.start(QLatin1String("gdb"), QStringList()
        << QLatin1String("--nw") // Do not use a window interface.
        << QLatin1String("--nx") // Do not read .gdbinit file.
        << QLatin1String("--batch") // Exit after processing options.
        << QLatin1String("--command") << createTemporaryCommandFile()
        << QLatin1String("--pid") << QString::number(pid)
    );
}

void BacktraceCollector::onDebuggerFinished(int exitCode, QProcess::ExitStatus /*exitStatus*/)
{
    if (d->errorOccurred) {
        emit error(QLatin1String("QProcess: ") + d->debugger.errorString());
        return;
    }
    if (exitCode != 0) {
        emit error(QString::fromLatin1("Debugger exited with code %1.").arg(exitCode));
        return;
    }
    emit backtrace(d->output);
}

void BacktraceCollector::onDebuggerError(QProcess::ProcessError /*error*/)
{
    d->errorOccurred = true;
}

QString BacktraceCollector::createTemporaryCommandFile()
{
    d->commandFile.reset(new QTemporaryFile);
    if (!d->commandFile->open()) {
        emit error(QLatin1String("Error: Could not create temporary command file."));
        return QString();
    }
    if (d->commandFile->write(GdbBatchCommands) == -1) {
        emit error(QLatin1String("Error: Could not write temporary command file."));
        return QString();
    }
    d->commandFile->close();
    return d->commandFile->fileName();
}

void BacktraceCollector::onDebuggerOutputAvailable()
{
    const QString newChunk = d->debugger.readAll();
    d->output.append(newChunk);
    emit backtraceChunk(newChunk);
}
