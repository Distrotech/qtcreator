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

#include "javaparser.h"

#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/task.h>

using namespace Android::Internal;
using namespace ProjectExplorer;

JavaParser::JavaParser() :
    ProjectExplorer::IOutputParser()
  , m_javaRegExp(QLatin1String("^(.*\\[javac\\]\\s)(.*\\.java):(\\d+):(.*)$"))
{
}

void JavaParser::stdOutput(const QString &line)
{
    stdError(line);
}

void JavaParser::stdError(const QString &line)
{
    if (m_javaRegExp.indexIn(line) > -1) {
        bool ok;
        int lineno = m_javaRegExp.cap(3).toInt(&ok);
        if (!ok)
            lineno = -1;
        QString file = m_javaRegExp.cap(2);
        for (int i = 0; i < m_fileList.size(); i++)
            if (m_fileList[i].endsWith(file)) {
                file = m_fileList[i];
                break;
            }

        Task task(Task::Error,
                  m_javaRegExp.cap(4).trimmed(),
                  Utils::FileName::fromString(file) /* filename */,
                  lineno,
                  ProjectExplorer::Constants::TASK_CATEGORY_COMPILE);
        emit addTask(task);
        return;
    }
    IOutputParser::stdError(line);
}

void JavaParser::setProjectFileList(const QStringList &fileList)
{
    m_fileList = fileList;
}
