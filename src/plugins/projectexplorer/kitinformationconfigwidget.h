/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
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

#ifndef KITINFORMATIONCONFIGWIDGET_H
#define KITINFORMATIONCONFIGWIDGET_H

#include "kitconfigwidget.h"

#include <coreplugin/id.h>

QT_BEGIN_NAMESPACE
class QComboBox;
class QPushButton;
QT_END_NAMESPACE

namespace Utils { class PathChooser; }

namespace ProjectExplorer {

class DeviceManagerModel;
class Kit;
class ToolChain;

namespace Internal {

// --------------------------------------------------------------------------
// SysRootInformationConfigWidget:
// --------------------------------------------------------------------------

class SysRootInformationConfigWidget : public KitConfigWidget
{
    Q_OBJECT

public:
    explicit SysRootInformationConfigWidget(Kit *k, QWidget *parent = 0);

    QString displayName() const;
    void apply();
    void discard();
    bool isDirty() const;
    void makeReadOnly();
    QWidget *buttonWidget() const;

private:
    Kit *m_kit;
    Utils::PathChooser *m_chooser;
};

// --------------------------------------------------------------------------
// ToolChainInformationConfigWidget:
// --------------------------------------------------------------------------

class ToolChainInformationConfigWidget : public KitConfigWidget
{
    Q_OBJECT

public:
    explicit ToolChainInformationConfigWidget(Kit *k, QWidget *parent = 0);

    QString displayName() const;
    void apply();
    void discard();
    bool isDirty() const;
    void makeReadOnly();
    QWidget *buttonWidget() const;

private slots:
    void toolChainAdded(ProjectExplorer::ToolChain *tc);
    void toolChainRemoved(ProjectExplorer::ToolChain *tc);
    void toolChainUpdated(ProjectExplorer::ToolChain *tc);
    void manageToolChains();

private:
    void updateComboBox();
    int indexOf(const ToolChain *tc);

    bool m_isReadOnly;
    Kit *m_kit;
    QComboBox *m_comboBox;
    QPushButton *m_manageButton;
};

// --------------------------------------------------------------------------
// DeviceTypeInformationConfigWidget:
// --------------------------------------------------------------------------

class DeviceTypeInformationConfigWidget : public KitConfigWidget
{
    Q_OBJECT

public:
    explicit DeviceTypeInformationConfigWidget(Kit *k, QWidget *parent = 0);

    QString displayName() const;
    void apply();
    void discard();
    bool isDirty() const;
    void makeReadOnly();

private:
    bool m_isReadOnly;
    Kit *m_kit;
    QComboBox *m_comboBox;
};

// --------------------------------------------------------------------------
// DeviceInformationConfigWidget:
// --------------------------------------------------------------------------

class DeviceInformationConfigWidget : public KitConfigWidget
{
    Q_OBJECT

public:
    explicit DeviceInformationConfigWidget(Kit *k, QWidget *parent = 0);

    QString displayName() const;
    void apply();
    void discard();
    bool isDirty() const;
    void makeReadOnly();
    QWidget *buttonWidget() const;

private slots:
    void manageDevices();
    void modelAboutToReset();
    void modelReset();

private:
    bool m_isReadOnly;
    Kit *m_kit;
    QComboBox *m_comboBox;
    QPushButton *m_manageButton;
    DeviceManagerModel *m_model;
    Core::Id m_selectedId;
};

} // namespace Internal
} // namespace ProjectExplorer

#endif // KITINFORMATIONCONFIGWIDGET_H
