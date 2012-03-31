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

#include "stackwindow.h"
#include "stackhandler.h"

#include "debuggeractions.h"
#include "debuggercore.h"
#include "debuggerengine.h"
#include "debuggerdialogs.h"
#include "memoryagent.h"

#include <utils/qtcassert.h>
#include <utils/savedaction.h>

#include <QDebug>

#include <QApplication>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QHeaderView>
#include <QInputDialog>
#include <QMenu>

namespace Debugger {
namespace Internal {

static DebuggerEngine *currentEngine()
{
    return debuggerCore()->currentEngine();
}

StackTreeView::StackTreeView(QWidget *parent)
    : BaseTreeView(parent)
{
    setWindowTitle(tr("Stack"));
    setAlwaysAdjustColumnsAction(debuggerCore()->action(AlwaysAdjustStackColumnWidths));

    connect(debuggerCore()->action(UseAddressInStackView), SIGNAL(toggled(bool)),
        SLOT(showAddressColumn(bool)));
    connect(debuggerCore()->action(ExpandStack), SIGNAL(triggered()),
        SLOT(reloadFullStack()));
    connect(debuggerCore()->action(MaximalStackDepth), SIGNAL(triggered()),
        SLOT(reloadFullStack()));
    showAddressColumn(false);
}

void StackTreeView::showAddressColumn(bool on)
{
    setColumnHidden(4, !on);
}

void StackTreeView::rowActivated(const QModelIndex &index)
{
    currentEngine()->activateFrame(index.row());
}

void StackTreeView::setModel(QAbstractItemModel *model)
{
    BaseTreeView::setModel(model);
    resizeColumnToContents(0);
    resizeColumnToContents(3);
    showAddressColumn(debuggerCore()->action(UseAddressInStackView)->isChecked());
}

// Input a function to be disassembled. Accept CDB syntax
// 'Module!function' for module specification

static inline StackFrame inputFunctionForDisassembly()
{
    StackFrame frame;
    QInputDialog dialog;
    dialog.setInputMode(QInputDialog::TextInput);
    dialog.setLabelText(StackTreeView::tr("Function:"));
    dialog.setWindowTitle(StackTreeView::tr("Disassemble Function"));
    dialog.setWindowFlags(dialog.windowFlags() & ~Qt::WindowContextHelpButtonHint);
    if (dialog.exec() != QDialog::Accepted)
        return frame;
    const QString function = dialog.textValue();
    if (function.isEmpty())
        return frame;
    const int bangPos = function.indexOf(QLatin1Char('!'));
    if (bangPos != -1) {
        frame.from = function.left(bangPos);
        frame.function = function.mid(bangPos + 1);
    } else {
        frame.function = function;
    }
    frame.line = 42; // trick gdb into mixed mode.
    return frame;
}

void StackTreeView::contextMenuEvent(QContextMenuEvent *ev)
{
    DebuggerEngine *engine = currentEngine();
    StackHandler *handler = engine->stackHandler();
    const QModelIndex index = indexAt(ev->pos());
    const int row = index.row();
    StackFrame frame;
    if (row >= 0 && row < handler->stackSize())
        frame = handler->frameAt(row);
    const quint64 address = frame.address;

    QMenu menu;
    menu.addAction(debuggerCore()->action(ExpandStack));

    QAction *actCopyContents = menu.addAction(tr("Copy Contents to Clipboard"));
    actCopyContents->setEnabled(model() != 0);

    if (engine->hasCapability(CreateFullBacktraceCapability))
        menu.addAction(debuggerCore()->action(CreateFullBacktrace));

    QAction *actShowMemory = 0;
    if (engine->hasCapability(ShowMemoryCapability)) {
        actShowMemory = menu.addAction(QString());
        if (address == 0) {
            actShowMemory->setText(tr("Open Memory Editor"));
            actShowMemory->setEnabled(false);
        } else {
            actShowMemory->setText(tr("Open Memory Editor at 0x%1").arg(address, 0, 16));
            actShowMemory->setEnabled(engine->hasCapability(ShowMemoryCapability));
        }
    }

    QAction *actShowDisassemblerAt = 0;
    QAction *actShowDisassemblerAtAddress = 0;
    QAction *actShowDisassemblerAtFunction = 0;

    if (engine->hasCapability(DisassemblerCapability)) {
        actShowDisassemblerAt = menu.addAction(QString());
        actShowDisassemblerAtAddress = menu.addAction(tr("Open Disassembler at Address..."));
        actShowDisassemblerAtFunction = menu.addAction(tr("Disassemble Function..."));
        if (address == 0) {
            actShowDisassemblerAt->setText(tr("Open Disassembler"));
            actShowDisassemblerAt->setEnabled(false);
        } else {
            actShowDisassemblerAt->setText(tr("Open Disassembler at 0x%1").arg(address, 0, 16));
        }
    }


    QAction *actLoadSymbols = 0;
    if (engine->hasCapability(ShowModuleSymbolsCapability))
        actLoadSymbols = menu.addAction(tr("Try to Load Unknown Symbols"));

#if 0 // @TODO: not implemented
    menu.addAction(debuggerCore()->action(UseToolTipsInStackView));
#endif
    if (engine->hasCapability(MemoryAddressCapability))
        menu.addAction(debuggerCore()->action(UseAddressInStackView));

    menu.addSeparator();

    addBaseContextActions(&menu);

    QAction *act = menu.exec(ev->globalPos());
    if (!act)
        return;

    if (act == actCopyContents)
        copyContentsToClipboard();
    else if (act == actShowMemory) {
        const QString title = tr("Memory at Frame #%1 (%2) 0x%3").
        arg(row).arg(frame.function).arg(address, 0, 16);
        QList<MemoryMarkup> ml;
        ml.push_back(MemoryMarkup(address, 1, QColor(Qt::blue).lighter(),
                                  tr("Frame #%1 (%2)").arg(row).arg(frame.function)));
        engine->openMemoryView(address, 0, ml, QPoint(), title);
    } else if (act == actShowDisassemblerAtAddress) {
        AddressDialog dialog;
        if (address)
            dialog.setAddress(address);
        if (dialog.exec() == QDialog::Accepted)
            currentEngine()->openDisassemblerView(Location(dialog.address()));
    } else if (act == actShowDisassemblerAtFunction) {
        const StackFrame frame = inputFunctionForDisassembly();
        if (!frame.function.isEmpty())
            currentEngine()->openDisassemblerView(Location(frame));
    } else if (act == actShowDisassemblerAt)
        engine->openDisassemblerView(frame);
    else if (act == actLoadSymbols)
        engine->loadSymbolsForStack();
    else
        handleBaseContextAction(act);
}

void StackTreeView::copyContentsToClipboard()
{
    QString str;
    int n = model()->rowCount();
    int m = model()->columnCount();
    for (int i = 0; i != n; ++i) {
        for (int j = 0; j != m; ++j) {
            QModelIndex index = model()->index(i, j);
            str += model()->data(index).toString();
            str += QLatin1Char('\t');
        }
        str += QLatin1Char('\n');
    }
    QClipboard *clipboard = QApplication::clipboard();
#    ifdef Q_WS_X11
    clipboard->setText(str, QClipboard::Selection);
#    endif
    clipboard->setText(str, QClipboard::Clipboard);
}

void StackTreeView::reloadFullStack()
{
    currentEngine()->reloadFullStack();
}

} // namespace Internal
} // namespace Debugger
