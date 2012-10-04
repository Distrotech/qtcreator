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
#ifndef PROJECTEXPLORER_DEPLOYMENTINFO_H
#define PROJECTEXPLORER_DEPLOYMENTINFO_H

#include "deployablefile.h"
#include "projectexplorer_export.h"

#include <QList>
#include <QSet>

namespace ProjectExplorer {

class PROJECTEXPLORER_EXPORT DeploymentData
{
public:
    void setFileList(const QList<DeployableFile> &files) { m_files = files; }

    void addFile(const DeployableFile &file)
    {
        if (!m_files.contains(file))
            m_files << file;
    }

    void addFile(const QString &localFilePath, const QString &remoteDirectory)
    {
        addFile(DeployableFile(localFilePath, remoteDirectory));
    }

    int fileCount() const { return m_files.count(); }
    DeployableFile fileAt(int index) const { return m_files.at(index); }
    QList<DeployableFile> allFiles() const { return m_files; }

    DeployableFile deployableForLocalFile(const QString &localFilePath) const
    {
        foreach (const DeployableFile &d, m_files) {
            if (d.localFilePath().toString() == localFilePath)
                return d;
        }
        return DeployableFile();
    }

    bool operator==(const DeploymentData &other) const
    {
        return m_files.toSet() == other.m_files.toSet();
    }

private:
    QList<DeployableFile> m_files;
};

inline bool operator!=(const DeploymentData &d1, const DeploymentData &d2)
{
    return !(d1 == d2);
}

} // namespace ProjectExplorer

#endif // PROJECTEXPLORER_DEPLOYMENTINFO_H
