/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#ifndef DEBUGGER_DEBUGGERKITINFORMATION_H
#define DEBUGGER_DEBUGGERKITINFORMATION_H

#include "debugger_global.h"
#include "debuggerconstants.h"

#include <projectexplorer/abi.h>
#include <projectexplorer/kitinformation.h>

namespace Debugger {

namespace Internal { class DebuggerItemManager; }

class DEBUGGER_EXPORT DebuggerItem
{
public:
    DebuggerItem();

    bool canClone() const { return true; }
    bool isValid() const;
    QString engineTypeName() const;

    QVariantMap toMap() const;
    void fromMap(const QVariantMap &data);
    void reinitializeFromFile();

    QVariant id() const { return m_id; }

    QString displayName() const { return m_displayName; }
    void setDisplayName(const QString &displayName);

    DebuggerEngineType engineType() const { return m_engineType; }
    void setEngineType(const DebuggerEngineType &engineType);

    Utils::FileName command() const { return m_command; }
    void setCommand(const Utils::FileName &command);

    bool isAutoDetected() const { return m_isAutoDetected; }
    void setAutoDetected(bool isAutoDetected);

    QList<ProjectExplorer::Abi> abis() const { return m_abis; }
    void setAbis(const QList<ProjectExplorer::Abi> &abis);
    void setAbi(const ProjectExplorer::Abi &abi);

    QStringList abiNames() const;

private:
    friend class Debugger::Internal::DebuggerItemManager;
    void setId(const QVariant &id);

    QVariant m_id;
    QString m_displayName;
    DebuggerEngineType m_engineType;
    Utils::FileName m_command;
    bool m_isAutoDetected;
    QList<ProjectExplorer::Abi> m_abis;
};

class DEBUGGER_EXPORT DebuggerKitInformation : public ProjectExplorer::KitInformation
{
    Q_OBJECT

public:
    DebuggerKitInformation();

    QVariant defaultValue(ProjectExplorer::Kit *k) const;

    QList<ProjectExplorer::Task> validate(const ProjectExplorer::Kit *k) const
        { return DebuggerKitInformation::validateDebugger(k); }

    void setup(ProjectExplorer::Kit *k);

    static QList<ProjectExplorer::Task> validateDebugger(const ProjectExplorer::Kit *k);
    static bool isValidDebugger(const ProjectExplorer::Kit *k);

    ProjectExplorer::KitConfigWidget *createConfigWidget(ProjectExplorer::Kit *k) const;

    ItemList toUserOutput(const ProjectExplorer::Kit *k) const;

    static void setDebugger(ProjectExplorer::Kit *k, const DebuggerItem &item);
    static void setDebugger(ProjectExplorer::Kit *k, const Utils::FileName &command);

    static Core::Id id();
    static Utils::FileName debuggerCommand(const ProjectExplorer::Kit *k);
    static DebuggerEngineType engineType(const ProjectExplorer::Kit *k);
    static QString displayString(const ProjectExplorer::Kit *k);
};

} // namespace Debugger

#endif // DEBUGGER_DEBUGGERKITINFORMATION_H
