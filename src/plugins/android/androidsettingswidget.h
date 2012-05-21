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

#ifndef ANDROIDSETTINGSWIDGET_H
#define ANDROIDSETTINGSWIDGET_H

#include "androidconfigurations.h"

#include <QList>
#include <QSharedPointer>
#include <QString>
#include <QWidget>
#include <QAbstractTableModel>

QT_BEGIN_NAMESPACE
class QLineEdit;
class QModelIndex;

class Ui_AndroidSettingsWidget;
QT_END_NAMESPACE

namespace Android {
namespace Internal {

class AVDModel: public QAbstractTableModel
{
    Q_OBJECT
public:
    void setAvdList(QVector<AndroidDevice> list);
    QString avdName(const QModelIndex &index);

protected:
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

private:
    QVector<AndroidDevice> m_list;
};

class AndroidSettingsWidget : public QWidget
{
    Q_OBJECT
public:
    AndroidSettingsWidget(QWidget *parent);
    ~AndroidSettingsWidget();

    void saveSettings(bool saveNow = false);
    QString searchKeywords() const;

private slots:
    void sdkLocationEditingFinished();
    void ndkLocationEditingFinished();
    void antLocationEditingFinished();
    void gdbLocationEditingFinished();
    void gdbserverLocationEditingFinished();
    void gdbLocationX86EditingFinished();
    void gdbserverLocationX86EditingFinished();
    void openJDKLocationEditingFinished();
    void browseSDKLocation();
    void browseNDKLocation();
    void browseAntLocation();
    void browseGdbLocation();
    void browseGdbserverLocation();
    void browseGdbLocationX86();
    void browseGdbserverLocationX86();
    void browseOpenJDKLocation();
    void toolchainVersionIndexChanged(QString);
    void addAVD();
    void removeAVD();
    void startAVD();
    void avdActivated(QModelIndex);
    void dataPartitionSizeEditingFinished();
    void manageAVD();

private:
    void initGui();
    bool checkSDK(const QString &location);
    bool checkNDK(const QString &location);
    void fillToolchainVersions();

    Ui_AndroidSettingsWidget *m_ui;
    AndroidConfig m_androidConfig;
    AVDModel m_AVDModel;
    bool m_saveSettingsRequested;
};

} // namespace Internal
} // namespace Android

#endif // ANDROIDSETTINGSWIDGET_H
