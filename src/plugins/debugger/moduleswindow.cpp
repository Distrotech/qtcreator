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

#include "moduleswindow.h"

#include "debuggeractions.h"
#include "debuggerconstants.h"
#include "debuggercore.h"
#include "debuggerengine.h"

#include <utils/qtcassert.h>
#include <utils/savedaction.h>

#include <QDebug>
#include <QProcess>

#include <QHeaderView>
#include <QMenu>
#include <QResizeEvent>


///////////////////////////////////////////////////////////////////////////
//
// ModulesWindow
//
///////////////////////////////////////////////////////////////////////////

namespace Debugger {
namespace Internal {

ModulesTreeView::ModulesTreeView(QWidget *parent)
  : BaseTreeView(parent)
{
    setSortingEnabled(true);
    setAlwaysAdjustColumnsAction(debuggerCore()->action(AlwaysAdjustModulesColumnWidths));

    connect(this, SIGNAL(activated(QModelIndex)),
        SLOT(moduleActivated(QModelIndex)));
}

void ModulesTreeView::moduleActivated(const QModelIndex &index)
{
    DebuggerEngine *engine = debuggerCore()->currentEngine();
    QTC_ASSERT(engine, return);
    if (index.isValid())
        engine->gotoLocation(index.sibling(index.row(), 1).data().toString());
}

void ModulesTreeView::contextMenuEvent(QContextMenuEvent *ev)
{
    QString name;
    QString fileName;
    QModelIndex index = indexAt(ev->pos());
    if (index.isValid())
        index = index.sibling(index.row(), 0);
    if (index.isValid()) {
        name = index.data().toString();
        fileName = index.sibling(index.row(), 1).data().toString();
    }

    DebuggerEngine *engine = debuggerCore()->currentEngine();
    QTC_ASSERT(engine, return);
    const bool enabled = engine->debuggerActionsEnabled();
    const bool canReload = engine->hasCapability(ReloadModuleCapability);
    const bool canLoadSymbols = engine->hasCapability(ReloadModuleSymbolsCapability);

    QMenu menu;

    QAction *actUpdateModuleList
        = new QAction(tr("Update Module List"), &menu);
    actUpdateModuleList->setEnabled(enabled && canReload);

    QAction *actShowModuleSources
        = new QAction(tr("Show Source Files for Module \"%1\"").arg(name), &menu);
    actShowModuleSources->setEnabled(enabled && canReload);

    QAction *actLoadSymbolsForAllModules
        = new QAction(tr("Load Symbols for All Modules"), &menu);
    actLoadSymbolsForAllModules->setEnabled(enabled && canLoadSymbols);

    QAction *actExamineAllModules
        = new QAction(tr("Examine All Modules"), &menu);
    actExamineAllModules->setEnabled(enabled && canLoadSymbols);

    QAction *actLoadSymbolsForModule = 0;
    QAction *actEditFile = 0;
    QAction *actShowModuleSymbols = 0;
    QAction *actShowDependencies = 0; // Show dependencies by running 'depends.exe'
    if (name.isEmpty()) {
        actLoadSymbolsForModule = new QAction(tr("Load Symbols for Module"), &menu);
        actLoadSymbolsForModule->setEnabled(false);
        actEditFile = new QAction(tr("Edit File"), &menu);
        actEditFile->setEnabled(false);
        actShowModuleSymbols = new QAction(tr("Show Symbols"), &menu);
        actShowModuleSymbols->setEnabled(false);
        actShowDependencies = new QAction(tr("Show Dependencies"), &menu);
        actShowDependencies->setEnabled(false);
    } else {
        actLoadSymbolsForModule
            = new QAction(tr("Load Symbols for Module \"%1\"").arg(name), &menu);
        actLoadSymbolsForModule->setEnabled(canLoadSymbols);
        actEditFile
            = new QAction(tr("Edit File \"%1\"").arg(name), &menu);
        actShowModuleSymbols
            = new QAction(tr("Show Symbols in File \"%1\"").arg(name), &menu);
        actShowModuleSymbols->setEnabled(engine->hasCapability(ShowModuleSymbolsCapability));
        actShowDependencies = new QAction(tr("Show Dependencies of \"%1\"").arg(name), &menu);
        actShowDependencies->setEnabled(!fileName.isEmpty());
#ifndef Q_OS_WIN
        // FIXME: Dependencies only available on Windows, when "depends" is installed.
        actShowDependencies->setEnabled(false);
#endif
    }

    menu.addAction(actUpdateModuleList);
    //menu.addAction(actShowModuleSources);  // FIXME
    menu.addAction(actShowDependencies);
    menu.addAction(actLoadSymbolsForAllModules);
    menu.addAction(actExamineAllModules);
    menu.addAction(actLoadSymbolsForModule);
    menu.addAction(actEditFile);
    menu.addAction(actShowModuleSymbols);
    addBaseContextActions(&menu);

    QAction *act = menu.exec(ev->globalPos());

    if (act == actUpdateModuleList)
        engine->reloadModules();
    else if (act == actShowModuleSources)
        engine->loadSymbols(fileName);
    else if (act == actLoadSymbolsForAllModules)
        engine->loadAllSymbols();
    else if (act == actExamineAllModules)
        engine->examineModules();
    else if (act == actLoadSymbolsForModule)
        engine->loadSymbols(fileName);
    else if (act == actEditFile)
        engine->gotoLocation(fileName);
    else if (act == actShowModuleSymbols)
        engine->requestModuleSymbols(fileName);
    else if (actShowDependencies && act == actShowDependencies)
        QProcess::startDetached(QLatin1String("depends"), QStringList(fileName));
    else
        handleBaseContextAction(act);
}

ModulesWindow::ModulesWindow()
    : BaseWindow(new ModulesTreeView)
{
    setWindowTitle(tr("Modules"));
}

} // namespace Internal
} // namespace Debugger
