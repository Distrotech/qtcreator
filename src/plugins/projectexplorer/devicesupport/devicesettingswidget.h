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

#ifndef DEVICESETTINGSWIDGET_H
#define DEVICESETTINGSWIDGET_H

#include <QList>
#include <QString>
#include <QPushButton>
#include <QWidget>

QT_BEGIN_NAMESPACE
class QSignalMapper;
QT_END_NAMESPACE

namespace ProjectExplorer {
class IDevice;
class DeviceManager;
class IDeviceWidget;

namespace Internal {
namespace Ui { class DeviceSettingsWidget; }
class NameValidator;

class DeviceSettingsWidget : public QWidget
{
    Q_OBJECT
public:
    DeviceSettingsWidget(QWidget *parent);
    ~DeviceSettingsWidget();

    void saveSettings();
    QString searchKeywords() const;

private slots:
    void currentDeviceChanged(int index);
    void addDevice();
    void removeDevice();
    void deviceNameEditingFinished();
    void setDefaultDevice();
    void handleAdditionalActionRequest(const QString &actionId);

private:
    void initGui();
    void displayCurrent();
    QSharedPointer<const IDevice> currentDevice() const;
    int currentIndex() const;
    void clearDetails();
    QString parseTestOutput();
    void fillInValues();

    Ui::DeviceSettingsWidget *m_ui;
    DeviceManager * const m_deviceManager;
    NameValidator * const m_nameValidator;
    bool m_saveSettingsRequested;
    QList<QPushButton *> m_additionalActionButtons;
    QSignalMapper * const m_additionalActionsMapper;
    IDeviceWidget *m_configWidget;
};

} // namespace Internal
} // namespace ProjectExplorer

#endif // DEVICESETTINGSWIDGET_H
