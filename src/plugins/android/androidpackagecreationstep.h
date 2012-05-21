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

#ifndef ANDROIDPACKAGECREATIONSTEP_H
#define ANDROIDPACKAGECREATIONSTEP_H

#include <projectexplorer/abi.h>
#include <projectexplorer/buildstep.h>
#include <QAbstractItemModel>
#include "javaparser.h"

QT_BEGIN_NAMESPACE
class QDateTime;
class QFile;
class QProcess;
QT_END_NAMESPACE

namespace Qt4ProjectManager {
class Qt4BuildConfiguration;
}

namespace Android {
namespace Internal {
class AndroidTarget;

class AndroidPackageCreationStep : public ProjectExplorer::BuildStep
{
    Q_OBJECT
    friend class AndroidPackageCreationFactory;
public:
    AndroidPackageCreationStep(ProjectExplorer::BuildStepList *bsl);
    ~AndroidPackageCreationStep();

    static bool removeDirectory(const QString &dirPath);

    static void stripAndroidLibs(const QStringList &files, ProjectExplorer::Abi::Architecture architecture);

    static const QLatin1String DefaultVersionNumber;

    AndroidTarget *androidTarget() const;

    void checkRequiredLibraries();
    void initCheckRequiredLibrariesForRun();
    void checkRequiredLibrariesForRun();

    QString keystorePath();
    void setKeystorePath(const QString &path);
    void setKeystorePassword(const QString &pwd);
    void setCertificateAlias(const QString &alias);
    void setCertificatePassword(const QString &pwd);
    void setOpenPackageLocation(bool open);
    QAbstractItemModel *keystoreCertificates();

protected:
    virtual bool fromMap(const QVariantMap &map);
    virtual QVariantMap toMap() const;

private slots:
    void handleBuildStdOutOutput();
    void handleBuildStdErrOutput();
    void keystorePassword();
    void certificatePassword();
    void showInGraphicalShell();
    void setQtLibs(const QStringList &qtLibs);
    void setPrebundledLibs(const QStringList &prebundledLibs);

private:
    AndroidPackageCreationStep(ProjectExplorer::BuildStepList *buildConfig,
                             AndroidPackageCreationStep *other);

    void ctor();
    virtual bool init();
    virtual void run(QFutureInterface<bool> &fi);
    virtual ProjectExplorer::BuildStepConfigWidget *createConfigWidget();
    virtual bool immutable() const { return true; }
    bool createPackage();
    bool runCommand(QProcess *buildProc, const QString &program, const QStringList &arguments);
    void raiseError(const QString &shortMsg,
                    const QString &detailedMsg = QString());

    static const Core::Id CreatePackageId;

private:
    QString m_keystorePath;
    QString m_keystorePasswd;
    QString m_certificateAlias;
    QString m_certificatePasswd;
    bool    m_openPackageLocation;
    JavaParser m_outputParser;

    // members to pass data from init() to run()
    QString m_androidDir;
    QString m_gdbServerSource;
    QString m_gdbServerDestination;
    bool m_debugBuild;
    QString m_antToolPath;
    QString m_apkPathUnsigned;
    QString m_apkPathSigned;
    QString m_keystorePathForRun;
    QString m_certificatePasswdForRun;
    QString m_jarSigner;
    // more for checkLibraries
    QString m_appPath;
    QString m_readElf;
    QStringList m_qtLibs;
    QStringList m_availableQtLibs;
    QStringList m_prebundledLibs;
signals:
    void updateRequiredLibrariesModels();
};

} // namespace Internal
} // namespace Android

#endif // ANDROIDPACKAGECREATIONSTEP_H
