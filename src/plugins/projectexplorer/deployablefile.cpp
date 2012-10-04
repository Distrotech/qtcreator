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

#include "deployablefile.h"

#include <QFileInfo>
#include <QHash>
#include <QPair>

using namespace Utils;

namespace ProjectExplorer {

DeployableFile::DeployableFile()
{
}

DeployableFile::DeployableFile(const QString &localFilePath, const QString &remoteDir)
    : m_localFilePath(FileName::fromUserInput(localFilePath)), m_remoteDir(remoteDir)
{
}

DeployableFile::DeployableFile(const FileName &localFilePath, const QString &remoteDir)
    : m_localFilePath(localFilePath), m_remoteDir(remoteDir)
{
}

QString DeployableFile::remoteFilePath() const
{
    return m_remoteDir.isEmpty()
            ? QString() : m_remoteDir + QLatin1Char('/') + m_localFilePath.toFileInfo().fileName();
}

bool DeployableFile::isValid() const
{
    return !m_localFilePath.toString().isEmpty() && !m_remoteDir.isEmpty();
}

uint qHash(const DeployableFile &d)
{
    return qHash(qMakePair(d.localFilePath().toString(), d.remoteDirectory()));
}

} // namespace ProjectExplorer
