/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2011 Nokia Corporation and/or its subsidiary(-ies).
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

#include "unconfiguredprojectpanel.h"
#include "wizards/targetsetuppage.h"
#include "qt4projectmanagerconstants.h"

#include "qt4project.h"
#include "qt4projectmanager.h"

#include <coreplugin/idocument.h>
#include <coreplugin/icore.h>
#include <coreplugin/modemanager.h>
#include <coreplugin/coreconstants.h>

#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/toolchain.h>

#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>

using namespace Qt4ProjectManager;
using namespace Qt4ProjectManager::Internal;


UnconfiguredProjectPanel::UnconfiguredProjectPanel()
{
}

QString Qt4ProjectManager::Internal::UnconfiguredProjectPanel::id() const
{
    return Constants::UNCONFIGURED_PANEL_PAGE_ID;
}

QString Qt4ProjectManager::Internal::UnconfiguredProjectPanel::displayName() const
{
    return tr("Configure Project");
}

int UnconfiguredProjectPanel::priority() const
{
    return -10;
}

bool Qt4ProjectManager::Internal::UnconfiguredProjectPanel::supports(ProjectExplorer::Project *project)
{
    if (qobject_cast<Qt4Project *>(project) && project->targets().isEmpty())
        return true;
    return false;
}

ProjectExplorer::PropertiesPanel *Qt4ProjectManager::Internal::UnconfiguredProjectPanel::createPanel(ProjectExplorer::Project *project)
{
    ProjectExplorer::PropertiesPanel *panel = new ProjectExplorer::PropertiesPanel;
    panel->setDisplayName(displayName());
    // TODO the icon needs a update
    panel->setIcon(QIcon(":/projectexplorer/images/unconfigured.png"));

    TargetSetupPageWrapper *w = new TargetSetupPageWrapper(project);
    panel->setWidget(w);
    return panel;
}

/////////
/// TargetSetupPageWrapper
////////

TargetSetupPageWrapper::TargetSetupPageWrapper(ProjectExplorer::Project *project)
    : QWidget(), m_project(qobject_cast<Qt4Project *>(project))
{
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setMargin(0);
    setLayout(layout);

    m_targetSetupPage = new TargetSetupPage(this);
    m_targetSetupPage->setUseScrollArea(false);
    m_targetSetupPage->setImportSearch(true);
    m_targetSetupPage->setProFilePath(project->document()->fileName());
    m_targetSetupPage->initializePage();
    m_targetSetupPage->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    updateNoteText();

    layout->addWidget(m_targetSetupPage);

    // Apply row
    QHBoxLayout *hbox = new QHBoxLayout();
    layout->addLayout(hbox);
    layout->setMargin(0);
    hbox->addStretch();
    m_configureButton = new QPushButton(this);
    m_configureButton->setText(tr("Configure Project"));
    hbox->addWidget(m_configureButton);

    layout->addStretch(10);

    completeChanged();

    connect(m_configureButton, SIGNAL(clicked()),
            this, SLOT(done()));
    connect(m_targetSetupPage, SIGNAL(noteTextLinkActivated()),
            this, SLOT(noteTextLinkActivated()));
    connect(m_targetSetupPage, SIGNAL(completeChanged()),
            this, SLOT(completeChanged()));
    connect(m_project->qt4ProjectManager(), SIGNAL(unconfiguredSettingsChanged()),
            this, SLOT(updateNoteText()));
}

void TargetSetupPageWrapper::updateNoteText()
{
    UnConfiguredSettings us = m_project->qt4ProjectManager()->unconfiguredSettings();

    QString text;
    if (us.version && us.toolchain)
        text = tr("<p>The project <b>%1</b> is not yet configured.</p><p>Qt Creator uses the Qt version: <b>%2</b> "
                  "and the tool chain: <b>%3</b> to parse the project. You can edit "
                  "these in the <b><a href=\"edit\">options.</a></b></p>")
                .arg(m_project->displayName())
                .arg(us.version->displayName())
                .arg(us.toolchain->displayName());
    else if (us.version)
        text = tr("<p>The project <b>%1</b> is not yet configured.</p><p>Qt Creator uses the Qt version: <b>%2</b> "
                  "and <b>no tool chain</b> to parse the project. You can edit "
                  "these in the <b><a href=\"edit\">settings</a></b></p>")
                .arg(m_project->displayName())
                .arg(us.version->displayName());
    else if (us.toolchain)
        text = tr("<p>The project <b>%1</b> is not yet configured.</p><p>Qt Creator uses <b>no Qt version</b> "
                  "and the tool chain: <b>%2</b> to parse the project. You can edit "
                  "these in the <b><a href=\"edit\">settings</a></b></p>")
                .arg(m_project->displayName())
                .arg(us.toolchain->displayName());
    else
        text = tr("<p>The project <b>%1</b> is not yet configured.</p><p>Qt Creator uses <b>no Qt version</b> "
                  "and <b>no tool chain</b> to parse the project. You can edit "
                  "these in the <b><a href=\"edit\">settings</a></b></p>")
                .arg(m_project->displayName());

    m_targetSetupPage->setNoteText(text);
}

void TargetSetupPageWrapper::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        event->accept();
        done();
    }
}

void TargetSetupPageWrapper::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        event->accept();
    }
}

void TargetSetupPageWrapper::done()
{
    m_targetSetupPage->setupProject(m_project);
    ProjectExplorer::ProjectExplorerPlugin::instance()->requestProjectModeUpdate(m_project);
    Core::ICore::instance()->modeManager()->activateMode(QLatin1String(Core::Constants::MODE_EDIT));
}

void TargetSetupPageWrapper::noteTextLinkActivated()
{
    Core::ICore::instance()->showOptionsDialog(QLatin1String(ProjectExplorer::Constants::PROJECTEXPLORER_SETTINGS_CATEGORY),
                                               QLatin1String(Constants::UNCONFIGURED_SETTINGS_PAGE_ID));
}

void TargetSetupPageWrapper::completeChanged()
{
    m_configureButton->setEnabled(m_targetSetupPage->isComplete());
}
