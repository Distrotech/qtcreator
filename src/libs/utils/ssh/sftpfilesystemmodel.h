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
#ifndef SFTPFILESYSTEMMODEL_H
#define SFTPFILESYSTEMMODEL_H

#include "sftpdefs.h"

#include <utils/utils_global.h>

#include <QAbstractItemModel>

namespace Utils {
class SshConnectionParameters;

namespace Internal { class SftpFileSystemModelPrivate; }

// Very simple read-only model. Symbolic links are not followed.
class QTCREATOR_UTILS_EXPORT SftpFileSystemModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    // Use this to get the full path of a file or directory via data().
    static const int PathRole = Qt::UserRole;

    explicit SftpFileSystemModel(QObject *parent = 0);
    ~SftpFileSystemModel();

    /*
     * Once this is called, an SFTP connection is established and the model is populated.
     * The effect of additional calls is undefined.
     */
    void setSshConnection(const SshConnectionParameters &sshParams);

    void setRootDirectory(const QString &path); // Default is "/".
    QString rootDirectory() const;

    SftpJobId downloadFile(const QModelIndex &index, const QString &targetFilePath);

signals:
     /*
      * E.g. "Permission denied". Note that this can happen without direct user intervention,
      * due to e.g. the view calling rowCount() on a non-readable directory. This signal should
      * therefore not result in a message box or similar, since it might occur very often.
      */
    void sftpOperationFailed(const QString &errorMessage);

    /*
     * This error is not recoverable. The model will not have any content after
     * the signal has been emitted.
     */
    void connectionError(const QString &errorMessage);

    // Success <=> error.isEmpty().
    void sftpOperationFinished(Utils::SftpJobId, const QString &error);

private slots:
    void handleSshConnectionEstablished();
    void handleSshConnectionFailure();
    void handleSftpChannelInitialized();
    void handleSftpChannelInitializationFailed(const QString &reason);
    void handleFileInfo(Utils::SftpJobId jobId, const QList<Utils::SftpFileInfo> &fileInfoList);
    void handleSftpJobFinished(Utils::SftpJobId jobId, const QString &errorMessage);

private:
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    void statRootDirectory();
    void shutDown();

    Internal::SftpFileSystemModelPrivate * const d;
};

} // namespace Utils;

#endif // SFTPFILESYSTEMMODEL_H
