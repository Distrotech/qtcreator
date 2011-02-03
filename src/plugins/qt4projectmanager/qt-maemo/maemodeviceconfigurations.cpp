/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of Qt Creator.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "maemodeviceconfigurations.h"

#include <coreplugin/icore.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QSettings>
#include <QtCore/QStringBuilder>
#include <QtGui/QDesktopServices>

#include <algorithm>
#include <cctype>

typedef Core::SshConnectionParameters::AuthType AuthType;

namespace Qt4ProjectManager {
namespace Internal {

namespace {
    const QLatin1String SettingsGroup("MaemoDeviceConfigs");
    const QLatin1String IdCounterKey("IdCounter");
    const QLatin1String ConfigListKey("ConfigList");
    const QLatin1String NameKey("Name");
    const QLatin1String OsVersionKey("OsVersion");
    const QLatin1String TypeKey("Type");
    const QLatin1String HostKey("Host");
    const QLatin1String SshPortKey("SshPort");
    const QLatin1String PortsSpecKey("FreePortsSpec");
    const QLatin1String UserNameKey("Uname");
    const QLatin1String AuthKey("Authentication");
    const QLatin1String KeyFileKey("KeyFile");
    const QLatin1String PasswordKey("Password");
    const QLatin1String TimeoutKey("Timeout");
    const QLatin1String IsDefaultKey("IsDefault");
    const QLatin1String InternalIdKey("InternalId");
    const QLatin1String DefaultKeyFilePathKey("DefaultKeyFile");

    const int DefaultSshPortHW(22);
    const int DefaultSshPortSim(6666);
    const int DefaultGdbServerPortHW(10000);
    const int DefaultGdbServerPortSim(13219);
    const QString DefaultHostNameHW(QLatin1String("192.168.2.15"));
    const QString DefaultHostNameSim(QLatin1String("localhost"));
    const AuthType DefaultAuthType(Core::SshConnectionParameters::AuthByKey);
    const int DefaultTimeout(30);
    const MaemoDeviceConfig::DeviceType DefaultDeviceType(MaemoDeviceConfig::Physical);
}

class DevConfNameMatcher
{
public:
    DevConfNameMatcher(const QString &name) : m_name(name) {}
    bool operator()(const MaemoDeviceConfig::ConstPtr &devConfig)
    {
        return devConfig->name() == m_name;
    }
private:
    const QString m_name;
};

class PortsSpecParser
{
    struct ParseException {
        ParseException(const char *error) : error(error) {}
        const char * const error;
    };

public:
    PortsSpecParser(const QString &portsSpec)
        : m_pos(0), m_portsSpec(portsSpec) { }

    /*
     * Grammar: Spec -> [ ElemList ]
     *          ElemList -> Elem [ ',' ElemList ]
     *          Elem -> Port [ '-' Port ]
     */
    MaemoPortList parse()
    {
        try {
            if (!atEnd())
                parseElemList();
        } catch (ParseException &e) {
            qWarning("Malformed ports specification: %s", e.error);
        }
        return m_portList;
    }

private:
    void parseElemList()
    {
        if (atEnd())
            throw ParseException("Element list empty.");
        parseElem();
        if (atEnd())
            return;
        if (nextChar() != ',') {
            throw ParseException("Element followed by something else "
                "than a comma.");
        }
        ++m_pos;
        parseElemList();
    }

    void parseElem()
    {
        const int startPort = parsePort();
        if (atEnd() || nextChar() != '-') {
            m_portList.addPort(startPort);
            return;
        }
        ++m_pos;
        const int endPort = parsePort();
        if (endPort < startPort)
            throw ParseException("Invalid range (end < start).");
        m_portList.addRange(startPort, endPort);
    }

    int parsePort()
    {
        if (atEnd())
            throw ParseException("Empty port string.");
        int port = 0;
        do {
            const char next = nextChar();
            if (!std::isdigit(next))
                break;
            port = 10*port + next - '0';
            ++m_pos;
        } while (!atEnd());
        if (port == 0 || port >= 2 << 16)
            throw ParseException("Invalid port value.");
        return port;
    }

    bool atEnd() const { return m_pos == m_portsSpec.length(); }
    char nextChar() const { return m_portsSpec.at(m_pos).toAscii(); }

    MaemoPortList m_portList;
    int m_pos;
    const QString &m_portsSpec;
};


void MaemoPortList::addPort(int port) { addRange(port, port); }

void MaemoPortList::addRange(int startPort, int endPort)
{
    m_ranges << Range(startPort, endPort);
}

bool MaemoPortList::hasMore() const { return !m_ranges.isEmpty(); }

int MaemoPortList::count() const
{
    int n = 0;
    foreach (const Range &r, m_ranges)
        n += r.second - r.first + 1;
    return n;
}

int MaemoPortList::getNext()
{
    Q_ASSERT(!m_ranges.isEmpty());
    Range &firstRange = m_ranges.first();
    const int next = firstRange.first++;
    if (firstRange.first > firstRange.second)
        m_ranges.removeFirst();
    return next;
}

QString MaemoPortList::toString() const
{
    QString stringRep;
    foreach (const Range &range, m_ranges) {
        stringRep += QString::number(range.first);
        if (range.second != range.first)
            stringRep += QLatin1Char('-') + QString::number(range.second);
        stringRep += QLatin1Char(',');
    }
    if (!stringRep.isEmpty())
        stringRep.remove(stringRep.length() - 1, 1); // Trailing comma.
    return stringRep;
}


MaemoDeviceConfig::Ptr MaemoDeviceConfig::create(const QString &name,
    MaemoGlobal::MaemoVersion osVersion, DeviceType type, Id &nextId)
{
    return Ptr(new MaemoDeviceConfig(name, osVersion, type, nextId));
}

MaemoDeviceConfig::Ptr MaemoDeviceConfig::create(const QSettings &settings,
    Id &nextId)
{
    return Ptr(new MaemoDeviceConfig(settings, nextId));
}

MaemoDeviceConfig::Ptr MaemoDeviceConfig::create(const ConstPtr &other)
{
    return Ptr(new MaemoDeviceConfig(other));
}

MaemoDeviceConfig::MaemoDeviceConfig(const QString &name,
    MaemoGlobal::MaemoVersion osVersion, DeviceType devType, Id &nextId)
    : m_sshParameters(Core::SshConnectionParameters::NoProxy),
      m_name(name),
      m_osVersion(osVersion),
      m_type(devType),
      m_portsSpec(defaultPortsSpec(m_type)),
      m_isDefault(false),
      m_internalId(nextId++)
{
    m_sshParameters.host = defaultHost(m_type);
    m_sshParameters.port = defaultSshPort(m_type);
    m_sshParameters.uname = defaultUser(m_osVersion);
    m_sshParameters.authType = DefaultAuthType;
    m_sshParameters.privateKeyFile
        = MaemoDeviceConfigurations::instance()->defaultSshKeyFilePath();
    m_sshParameters.timeout = DefaultTimeout;
}

MaemoDeviceConfig::MaemoDeviceConfig(const QSettings &settings,
        Id &nextId)
    : m_sshParameters(Core::SshConnectionParameters::NoProxy),
      m_name(settings.value(NameKey).toString()),
      m_osVersion(static_cast<MaemoGlobal::MaemoVersion>(settings.value(OsVersionKey, MaemoGlobal::Maemo5).toInt())),
      m_type(static_cast<DeviceType>(settings.value(TypeKey, DefaultDeviceType).toInt())),
      m_portsSpec(settings.value(PortsSpecKey, defaultPortsSpec(m_type)).toString()),
      m_isDefault(settings.value(IsDefaultKey, false).toBool()),
      m_internalId(settings.value(InternalIdKey, nextId).toULongLong())
{
    if (m_internalId == nextId)
        ++nextId;
    m_sshParameters.host = settings.value(HostKey, defaultHost(m_type)).toString();
    m_sshParameters.port = settings.value(SshPortKey, defaultSshPort(m_type)).toInt();
    m_sshParameters.uname = settings.value(UserNameKey, defaultUser(m_osVersion)).toString();
    m_sshParameters.authType
        = static_cast<AuthType>(settings.value(AuthKey, DefaultAuthType).toInt());
    m_sshParameters.pwd = settings.value(PasswordKey).toString();
    m_sshParameters.privateKeyFile
        = settings.value(KeyFileKey, defaultPrivateKeyFilePath()).toString();
    m_sshParameters.timeout = settings.value(TimeoutKey, DefaultTimeout).toInt();
}

MaemoDeviceConfig::MaemoDeviceConfig(const MaemoDeviceConfig::ConstPtr &other)
    : m_sshParameters(other->m_sshParameters),
      m_name(other->m_name),
      m_osVersion(other->m_osVersion),
      m_type(other->type()),
      m_portsSpec(other->m_portsSpec),
      m_isDefault(other->m_isDefault),
      m_internalId(other->m_internalId)
{
}

QString MaemoDeviceConfig::portsRegExpr()
{
    const QLatin1String portExpr("(\\d)+");
    const QString listElemExpr = QString::fromLatin1("%1(-%1)?").arg(portExpr);
    return QString::fromLatin1("((%1)(,%1)*)?").arg(listElemExpr);
}

int MaemoDeviceConfig::defaultSshPort(DeviceType type)
{
    return type == Physical ? DefaultSshPortHW : DefaultSshPortSim;
}

QString MaemoDeviceConfig::defaultPortsSpec(DeviceType type) const
{
    return QLatin1String(type == Physical ? "10000-10100" : "13219,14168");
}

QString MaemoDeviceConfig::defaultHost(DeviceType type)
{
    return type == Physical ? DefaultHostNameHW : DefaultHostNameSim;
}

QString MaemoDeviceConfig::defaultPrivateKeyFilePath()
{
    return QDesktopServices::storageLocation(QDesktopServices::HomeLocation)
        + QLatin1String("/.ssh/id_rsa");
}

QString MaemoDeviceConfig::defaultUser(MaemoGlobal::MaemoVersion osVersion)
{
    switch (osVersion) {
    case MaemoGlobal::Maemo5:
    case MaemoGlobal::Maemo6:
        return QLatin1String("developer");
    case MaemoGlobal::Meego:
        return QLatin1String("meego");
    default:
        Q_ASSERT(false);
        return QString();
    }
}

MaemoPortList MaemoDeviceConfig::freePorts() const
{
    return PortsSpecParser(m_portsSpec).parse();
}

void MaemoDeviceConfig::save(QSettings &settings) const
{
    settings.setValue(NameKey, m_name);
    settings.setValue(OsVersionKey, m_osVersion);
    settings.setValue(TypeKey, m_type);
    settings.setValue(HostKey, m_sshParameters.host);
    settings.setValue(SshPortKey, m_sshParameters.port);
    settings.setValue(PortsSpecKey, m_portsSpec);
    settings.setValue(UserNameKey, m_sshParameters.uname);
    settings.setValue(AuthKey, m_sshParameters.authType);
    settings.setValue(PasswordKey, m_sshParameters.pwd);
    settings.setValue(KeyFileKey, m_sshParameters.privateKeyFile);
    settings.setValue(TimeoutKey, m_sshParameters.timeout);
    settings.setValue(IsDefaultKey, m_isDefault);
    settings.setValue(InternalIdKey, m_internalId);
}

const MaemoDeviceConfig::Id MaemoDeviceConfig::InvalidId = 0;


MaemoDeviceConfigurations *MaemoDeviceConfigurations::instance(QObject *parent)
{
    if (m_instance == 0) {
        m_instance = new MaemoDeviceConfigurations(parent);
        m_instance->load();
    }
    return m_instance;
}

void MaemoDeviceConfigurations::replaceInstance(const MaemoDeviceConfigurations *other)
{
    Q_ASSERT(m_instance);
    m_instance->beginResetModel();
    copy(other, m_instance, false);
    m_instance->save();
    m_instance->endResetModel();
    emit m_instance->updated();
}

MaemoDeviceConfigurations *MaemoDeviceConfigurations::cloneInstance()
{
    MaemoDeviceConfigurations * const other = new MaemoDeviceConfigurations(0);
    copy(m_instance, other, true);
    return other;
}

void MaemoDeviceConfigurations::copy(const MaemoDeviceConfigurations *source,
    MaemoDeviceConfigurations *target, bool deep)
{
    if (deep) {
        foreach (const MaemoDeviceConfig::ConstPtr &devConf, source->m_devConfigs)
            target->m_devConfigs << MaemoDeviceConfig::create(devConf);
    } else {
        target->m_devConfigs = source->m_devConfigs;
    }
    target->m_defaultSshKeyFilePath = source->m_defaultSshKeyFilePath;
    target->m_nextId = source->m_nextId;
    target->initShadowDevConfs();
}

void MaemoDeviceConfigurations::save()
{
    QSettings *settings = Core::ICore::instance()->settings();
    settings->beginGroup(SettingsGroup);
    settings->setValue(IdCounterKey, m_nextId);
    settings->setValue(DefaultKeyFilePathKey, m_defaultSshKeyFilePath);
    settings->beginWriteArray(ConfigListKey, m_devConfigs.count());
    for (int i = 0; i < m_devConfigs.count(); ++i) {
        settings->setArrayIndex(i);
        m_devConfigs.at(i)->save(*settings);
    }
    settings->endArray();
    settings->endGroup();
}

void MaemoDeviceConfigurations::initShadowDevConfs()
{
    m_shadowDevConfigs.clear();
    for (int i = 0; i < m_devConfigs.count(); ++i)
        m_shadowDevConfigs.push_back(MaemoDeviceConfig::Ptr());
}

void MaemoDeviceConfigurations::setupShadowDevConf(int idx)
{
    MaemoDeviceConfig::Ptr shadowConf = m_shadowDevConfigs.at(idx);
    if (shadowConf)
        return;

    const MaemoDeviceConfig::Ptr devConf = m_devConfigs.at(idx);
    const MaemoDeviceConfig::DeviceType shadowType
        = devConf->type() == MaemoDeviceConfig::Physical
            ? MaemoDeviceConfig::Simulator : MaemoDeviceConfig::Physical;
    shadowConf = MaemoDeviceConfig::create(devConf->name(),
        devConf->osVersion(), shadowType, m_nextId);
    shadowConf->m_sshParameters.authType = devConf->m_sshParameters.authType;
    shadowConf->m_sshParameters.timeout = devConf->m_sshParameters.timeout;
    shadowConf->m_sshParameters.pwd = devConf->m_sshParameters.pwd;
    shadowConf->m_sshParameters.privateKeyFile
        = devConf->m_sshParameters.privateKeyFile;
    shadowConf->m_isDefault = devConf->m_isDefault;
    shadowConf->m_internalId = devConf->m_internalId;
    m_shadowDevConfigs[idx] = shadowConf;
}

void MaemoDeviceConfigurations::addConfiguration(const QString &name,
    MaemoGlobal::MaemoVersion osVersion, MaemoDeviceConfig::DeviceType type)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    const MaemoDeviceConfig::Ptr devConf
        = MaemoDeviceConfig::create(name, osVersion, type, m_nextId);
    if (m_devConfigs.isEmpty())
        devConf->m_isDefault = true;
    m_devConfigs << devConf;
    m_shadowDevConfigs << MaemoDeviceConfig::Ptr();
    endInsertRows();
}

void MaemoDeviceConfigurations::removeConfiguration(int idx)
{
    Q_ASSERT(idx >= 0 && idx < rowCount());
    beginRemoveRows(QModelIndex(), idx, idx);
    const bool wasDefault = deviceAt(idx)->m_isDefault;
    m_devConfigs.removeAt(idx);
    m_shadowDevConfigs.removeAt(idx);
    endRemoveRows();
    if (wasDefault && !m_devConfigs.isEmpty()) {
        m_devConfigs.first()->m_isDefault = true;
        const QModelIndex changedIndex = index(0, 0);
        emit dataChanged(changedIndex, changedIndex);
    }
}

void MaemoDeviceConfigurations::setConfigurationName(int i, const QString &name)
{
    Q_ASSERT(i >= 0 && i < rowCount());
    m_devConfigs.at(i)->m_name = name;
    const MaemoDeviceConfig::Ptr shadowConfig = m_shadowDevConfigs.at(i);
    if (shadowConfig)
        shadowConfig->m_name = name;
    const QModelIndex changedIndex = index(i, 0);
    emit dataChanged(changedIndex, changedIndex);
}

void MaemoDeviceConfigurations::setDeviceType(int i,
    const MaemoDeviceConfig::DeviceType type)
{
    Q_ASSERT(i >= 0 && i < rowCount());
    MaemoDeviceConfig::Ptr &current = m_devConfigs[i];
    if (current->type() == type)
        return;
    setupShadowDevConf(i);
    std::swap(current, m_shadowDevConfigs[i]);
}

void MaemoDeviceConfigurations::setSshParameters(int i,
    const Core::SshConnectionParameters &params)
{
    Q_ASSERT(i >= 0 && i < rowCount());
    m_devConfigs.at(i)->m_sshParameters = params;
}

void MaemoDeviceConfigurations::setPortsSpec(int i, const QString &portsSpec)
{
    Q_ASSERT(i >= 0 && i < rowCount());
    m_devConfigs.at(i)->m_portsSpec = portsSpec;
}

void MaemoDeviceConfigurations::setDefaultDevice(int idx)
{
    Q_ASSERT(idx >= 0 && idx < rowCount());
    if (m_devConfigs.at(idx)->m_isDefault)
        return;
    QModelIndex oldDefaultIndex;
    for (int i = 0; i < m_devConfigs.count(); ++i) {
        const MaemoDeviceConfig::Ptr &oldDefaultDev = m_devConfigs.at(i);
        if (oldDefaultDev->m_isDefault) {
            oldDefaultDev->m_isDefault = false;
            oldDefaultIndex = index(i, 0);
            break;
        }
    }
    Q_ASSERT(oldDefaultIndex.isValid());
    emit dataChanged(oldDefaultIndex, oldDefaultIndex);
    m_devConfigs.at(idx)->m_isDefault = true;
    const QModelIndex newDefaultIndex = index(idx, 0);
    emit dataChanged(newDefaultIndex, newDefaultIndex);
}

MaemoDeviceConfigurations::MaemoDeviceConfigurations(QObject *parent)
    : QAbstractListModel(parent)
{
}

void MaemoDeviceConfigurations::load()
{
    QSettings *settings = Core::ICore::instance()->settings();
    settings->beginGroup(SettingsGroup);
    m_nextId = settings->value(IdCounterKey, 1).toULongLong();
    m_defaultSshKeyFilePath = settings->value(DefaultKeyFilePathKey,
        MaemoDeviceConfig::defaultPrivateKeyFilePath()).toString();
    int count = settings->beginReadArray(ConfigListKey);
    bool hasDefault = false;
    for (int i = 0; i < count; ++i) {
        settings->setArrayIndex(i);
        MaemoDeviceConfig::Ptr devConf
            = MaemoDeviceConfig::create(*settings, m_nextId);
        hasDefault |= devConf->m_isDefault;
        m_devConfigs << devConf;
    }
    settings->endArray();
    settings->endGroup();
    initShadowDevConfs();
    if (!hasDefault && !m_devConfigs.isEmpty())
        m_devConfigs.first()->m_isDefault = true;
}

MaemoDeviceConfig::ConstPtr MaemoDeviceConfigurations::deviceAt(int idx) const
{
    Q_ASSERT(idx >= 0 && idx < rowCount());
    return m_devConfigs.at(idx);
}

bool MaemoDeviceConfigurations::hasConfig(const QString &name) const
{
    QList<MaemoDeviceConfig::Ptr>::ConstIterator resultIt =
        std::find_if(m_devConfigs.constBegin(), m_devConfigs.constEnd(),
            DevConfNameMatcher(name));
    return resultIt != m_devConfigs.constEnd();
}

MaemoDeviceConfig::ConstPtr MaemoDeviceConfigurations::find(MaemoDeviceConfig::Id id) const
{
    const int index = indexForInternalId(id);
    return index == -1 ? MaemoDeviceConfig::ConstPtr() : deviceAt(index);
}

MaemoDeviceConfig::ConstPtr MaemoDeviceConfigurations::defaultDeviceConfig() const
{
    foreach (const MaemoDeviceConfig::ConstPtr &devConf, m_devConfigs) {
        if (devConf->m_isDefault)
            return devConf;
    }
    Q_ASSERT(m_devConfigs.isEmpty());
    return MaemoDeviceConfig::ConstPtr();
}

int MaemoDeviceConfigurations::indexForInternalId(MaemoDeviceConfig::Id internalId) const
{
    for (int i = 0; i < m_devConfigs.count(); ++i) {
        if (deviceAt(i)->m_internalId == internalId)
            return i;
    }
    return -1;
}

MaemoDeviceConfig::Id MaemoDeviceConfigurations::internalId(MaemoDeviceConfig::ConstPtr devConf) const
{
    return devConf ? devConf->m_internalId : MaemoDeviceConfig::InvalidId;
}

int MaemoDeviceConfigurations::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_devConfigs.count();
}

QVariant MaemoDeviceConfigurations::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= rowCount() || role != Qt::DisplayRole)
        return QVariant();
    const MaemoDeviceConfig::ConstPtr devConf = deviceAt(index.row());
    QString name = devConf->name();
    if (devConf->m_isDefault)
        name += tr(" (default)");
    return name;
}

MaemoDeviceConfigurations *MaemoDeviceConfigurations::m_instance = 0;

} // namespace Internal
} // namespace Qt4ProjectManager
