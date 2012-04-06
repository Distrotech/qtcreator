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
#include "devicesettingswidget.h"
#include "ui_devicesettingswidget.h"

#include "devicefactoryselectiondialog.h"
#include "devicemanager.h"
#include "devicemanagermodel.h"
#include "idevice.h"
#include "idevicefactory.h"
#include "idevicewidget.h"
#include "idevicewizard.h"

#include <coreplugin/icore.h>
#include <extensionsystem/pluginmanager.h>
#include <utils/portlist.h>

#include <QFileInfo>
#include <QRegExp>
#include <QSettings>
#include <QSignalMapper>
#include <QTextStream>

#include <QFileDialog>
#include <QMessageBox>
#include <QIntValidator>

#include <algorithm>

using namespace Core;
using namespace Utils;

namespace ProjectExplorer {
namespace Internal {
const char LastDeviceIndexKey[] = "LastDisplayedMaemoDeviceConfig";

class NameValidator : public QValidator
{
public:
    NameValidator(const DeviceManager *deviceManager, QWidget *parent = 0)
        : QValidator(parent), m_deviceManager(deviceManager)
    {
    }

    void setDisplayName(const QString &name) { m_oldName = name; }

    virtual State validate(QString &input, int & /* pos */) const
    {
        if (input.trimmed().isEmpty()
                || (input != m_oldName && m_deviceManager->hasDevice(input)))
            return Intermediate;
        return Acceptable;
    }

    virtual void fixup(QString &input) const
    {
        int dummy = 0;
        if (validate(input, dummy) != Acceptable)
            input = m_oldName;
    }

private:
    QString m_oldName;
    const DeviceManager * const m_deviceManager;
};


DeviceSettingsWidget::DeviceSettingsWidget(QWidget *parent)
    : QWidget(parent),
      m_ui(new Ui::DeviceSettingsWidget),
      m_deviceManager(DeviceManager::cloneInstance()),
      m_nameValidator(new NameValidator(m_deviceManager, this)),
      m_saveSettingsRequested(false),
      m_additionalActionsMapper(new QSignalMapper(this)),
      m_configWidget(0)
{
    initGui();
    connect(m_additionalActionsMapper, SIGNAL(mapped(QString)),
        SLOT(handleAdditionalActionRequest(QString)));
}

DeviceSettingsWidget::~DeviceSettingsWidget()
{
    if (m_saveSettingsRequested) {
        Core::ICore::settings()->setValue(QLatin1String(LastDeviceIndexKey),
            currentIndex());
        DeviceManager::replaceInstance();
    }
    DeviceManager::removeClonedInstance();
    delete m_ui;
}

QString DeviceSettingsWidget::searchKeywords() const
{
    QString rc;
    QTextStream(&rc) << m_ui->configurationLabel->text()
        << ' ' << m_ui->deviceNameLabel->text()
        << ' ' << m_ui->nameLineEdit->text();
    if (m_configWidget)
    rc.remove(QLatin1Char('&'));
    return rc;
}

void DeviceSettingsWidget::initGui()
{
    m_ui->setupUi(this);
    DeviceManagerModel * const model = new DeviceManagerModel(m_deviceManager, this);
    m_ui->configurationComboBox->setModel(model);
    m_ui->nameLineEdit->setValidator(m_nameValidator);

    int lastIndex = Core::ICore::settings()
        ->value(QLatin1String(LastDeviceIndexKey), 0).toInt();
    if (lastIndex == -1)
        lastIndex = 0;
    if (lastIndex < m_ui->configurationComboBox->count())
        m_ui->configurationComboBox->setCurrentIndex(lastIndex);
    connect(m_ui->configurationComboBox, SIGNAL(currentIndexChanged(int)),
        SLOT(currentDeviceChanged(int)));
    currentDeviceChanged(currentIndex());
    connect(m_ui->defaultDeviceButton, SIGNAL(clicked()),
        SLOT(setDefaultDevice()));
}

void DeviceSettingsWidget::addDevice()
{
    const QList<IDeviceFactory *> &factories
        = ExtensionSystem::PluginManager::instance()->getObjects<IDeviceFactory>();

    if (factories.isEmpty()) // Can't happen, because this plugin provides the generic one.
        return;

    DeviceFactorySelectionDialog d;
    if (d.exec() != QDialog::Accepted)
        return;

    const QScopedPointer<IDeviceWizard> wizard(d.selectedFactory()->createWizard(this));
    if (wizard->exec() != QDialog::Accepted)
        return;

    m_deviceManager->addDevice(wizard->device());
    m_ui->removeConfigButton->setEnabled(true);
    m_ui->configurationComboBox->setCurrentIndex(m_ui->configurationComboBox->count()-1);
}

void DeviceSettingsWidget::removeDevice()
{
    m_deviceManager->removeDevice(currentIndex());
    if (m_deviceManager->deviceCount() == 0)
        currentDeviceChanged(-1);
}

void DeviceSettingsWidget::displayCurrent()
{
    const IDevice::ConstPtr &current = currentDevice();
    m_ui->defaultDeviceButton->setEnabled(
        m_deviceManager->defaultDevice(current->type()) != current);
    m_ui->osTypeValueLabel->setText(DeviceManager::displayNameForDeviceType(current->type()));
    m_ui->autoDetectionValueLabel->setText(current->isAutoDetected()
        ? tr("Yes (fingerprint is '%1')").arg(current->fingerprint()) : tr("No"));
    m_nameValidator->setDisplayName(current->displayName());
    m_ui->removeConfigButton->setEnabled(!current->isAutoDetected());
    fillInValues();
}

void DeviceSettingsWidget::fillInValues()
{
    const IDevice::ConstPtr &current = currentDevice();
    m_ui->nameLineEdit->setText(current->displayName());
}

void DeviceSettingsWidget::saveSettings()
{
    // We must defer this step because of a stupid bug on MacOS. See QTCREATORBUG-1675.
    m_saveSettingsRequested = true;
}

int DeviceSettingsWidget::currentIndex() const
{
    return m_ui->configurationComboBox->currentIndex();
}

QSharedPointer<const IDevice> DeviceSettingsWidget::currentDevice() const
{
    Q_ASSERT(currentIndex() != -1);
    return m_deviceManager->deviceAt(currentIndex());
}

void DeviceSettingsWidget::deviceNameEditingFinished()
{
    if (m_ui->configurationComboBox->count() == 0)
        return;

    const QString &newName = m_ui->nameLineEdit->text();
    m_deviceManager->setDeviceDisplayName(currentIndex(), newName);
    m_nameValidator->setDisplayName(newName);
}

void DeviceSettingsWidget::setDefaultDevice()
{
    m_deviceManager->setDefaultDevice(currentIndex());
    m_ui->defaultDeviceButton->setEnabled(false);
}

void DeviceSettingsWidget::currentDeviceChanged(int index)
{
    qDeleteAll(m_additionalActionButtons);
    delete m_configWidget;
    m_configWidget = 0;
    m_additionalActionButtons.clear();
    m_ui->generalGroupBox->setEnabled(false);
    m_ui->osSpecificGroupBox->setEnabled(false);
    if (index == -1) {
        m_ui->removeConfigButton->setEnabled(false);
        clearDetails();
        m_ui->defaultDeviceButton->setEnabled(false);
    } else {
        m_ui->removeConfigButton->setEnabled(true);
        const IDeviceFactory * const factory = factoryForCurrentDevice();
        if (factory) {
            const QStringList &actionIds = factory->supportedDeviceActionIds();
            foreach (const QString &actionId, actionIds) {
                QPushButton * const button = new QPushButton(
                            factory->displayNameForActionId(actionId));
                m_additionalActionButtons << button;
                connect(button, SIGNAL(clicked()), m_additionalActionsMapper, SLOT(map()));
                m_additionalActionsMapper->setMapping(button, actionId);
                m_ui->buttonsLayout->insertWidget(m_ui->buttonsLayout->count() - 1, button);
            }
            if (!m_ui->osSpecificGroupBox->layout())
                new QVBoxLayout(m_ui->osSpecificGroupBox);
            m_configWidget = factory->createWidget(m_deviceManager->mutableDeviceAt(currentIndex()),
                                                   m_ui->osSpecificGroupBox);
            if (m_configWidget) {
                m_ui->osSpecificGroupBox->layout()->addWidget(m_configWidget);
                m_ui->osSpecificGroupBox->setEnabled(factory->isUserEditable());
            }
            m_ui->generalGroupBox->setEnabled(factory->isUserEditable());
        }
        m_ui->configurationComboBox->setCurrentIndex(index);
        displayCurrent();
    }
}

void DeviceSettingsWidget::clearDetails()
{
    m_ui->nameLineEdit->clear();
    m_ui->osTypeValueLabel->clear();
    m_ui->autoDetectionValueLabel->clear();
}

const IDeviceFactory *DeviceSettingsWidget::factoryForCurrentDevice() const
{
    Q_ASSERT(currentDevice());
    return DeviceManager::factoryForDeviceType(currentDevice()->type());
}

void DeviceSettingsWidget::handleAdditionalActionRequest(const QString &actionId)
{
    const IDeviceFactory * const factory = factoryForCurrentDevice();
    Q_ASSERT(factory);
    QDialog * const action = factory->createDeviceAction(actionId, currentDevice(), this);
    if (action)
        action->exec();
    delete action;
}

} // namespace Internal
} // namespace ProjectExplorer
