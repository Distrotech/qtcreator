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

#include "qtmessagelogeditor.h"
#include "qtmessageloghandler.h"
#include "debuggerstringutils.h"
#include "debuggercore.h"
#include "debuggerengine.h"

#include <utils/qtcassert.h>

#include <QUrl>
#include <QMenu>
#include <QKeyEvent>

namespace Debugger {
namespace Internal {

///////////////////////////////////////////////////////////////////////
//
// QtMessageLogEditor
//
///////////////////////////////////////////////////////////////////////

QtMessageLogEditor::QtMessageLogEditor(const QModelIndex &index,
                             QWidget *parent) :
    QTextEdit(parent),
    m_historyIndex(index),
    m_prompt(_(":/debugger/images/prompt.png")),
    m_startOfEditableArea(0)
{
    setFrameStyle(QFrame::NoFrame);
    setUndoRedoEnabled(false);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    document()->addResource(QTextDocument::ImageResource,
                            QUrl(_("prompt")), m_prompt);
    QTextImageFormat format;
    format.setName(_("prompt"));
    format.setHeight(9);
    format.setWidth(9);
    textCursor().insertImage(format);
    textCursor().insertText(_("  "));
    m_startOfEditableArea = textCursor().position();

    ensureCursorVisible();
    setTextInteractionFlags(Qt::TextEditorInteraction);
}

void QtMessageLogEditor::keyPressEvent(QKeyEvent *e)
{
    bool keyConsumed = false;

    switch (e->key()) {
    case Qt::Key_Return:
    case Qt::Key_Enter: {
        keyConsumed = debuggerCore()->evaluateScriptExpression(getCurrentScript());
        if (keyConsumed) {
            emit editingFinished();
            debuggerCore()->currentEngine()->qtMessageLogHandler()->appendEditableRow();
        }
    }
        break;

    case Qt::Key_Backspace:
        if (textCursor().selectionStart() <= m_startOfEditableArea)
            keyConsumed = true;
        break;

    case Qt::Key_Delete:
        if (textCursor().selectionStart() < m_startOfEditableArea)
            keyConsumed = true;
        break;

    case Qt::Key_Home:
    {
        QTextCursor c(textCursor());
        c.setPosition(m_startOfEditableArea);
        setTextCursor(c);
        keyConsumed = true;
    }
        break;

    case Qt::Key_Up:
        handleUpKey();
        keyConsumed = true;
        break;

    case Qt::Key_Down:
        handleDownKey();
        keyConsumed = true;
        break;

        //  Ctrl+Left: Moves the cursor one word to the left.
        //  Left: Moves the cursor one character to the left.
    case Qt::Key_Left:
        if (textCursor().position() <= m_startOfEditableArea
                || e->modifiers() & Qt::ControlModifier) {
            QTextCursor c(textCursor());
            c.setPosition(m_startOfEditableArea);
            setTextCursor(c);
            keyConsumed = true;
        }
        break;

        //  Ctrl+Right: Moves the cursor one word to the right.
        //  Right: Moves the cursor one character to the right.
    case Qt::Key_Right:
        if ( !(e->modifiers() & Qt::ControlModifier)
             && textCursor().position() <= m_startOfEditableArea) {
            QTextCursor c(textCursor());
            c.setPosition(m_startOfEditableArea);
            setTextCursor(c);
            keyConsumed = true;
        }
        break;

        //  Ctrl+C, Ctrl+Insert: Allow to Copy the selected text to the clipboard.
    case Qt::Key_C:
    case Qt::Key_Insert:
        if (textCursor().selectionStart() < m_startOfEditableArea &&
                !(e->modifiers() & Qt::ControlModifier))
            keyConsumed = true;
        break;

    default:
        //      Disallow any other keys in the prompt area
        if (textCursor().selectionStart() < m_startOfEditableArea)
            keyConsumed = true;
        break;
    }

    if (!keyConsumed)
        QTextEdit::keyPressEvent(e);
}

void QtMessageLogEditor::contextMenuEvent(QContextMenuEvent *event)
{
    //TODO:: on right click the editor closes
    //FIXIT
    return QTextEdit::contextMenuEvent(event);

    QTextCursor cursor = textCursor();
    bool editable = cursor.position() > m_startOfEditableArea;
    QMenu *menu = new QMenu();
    QAction *a;

    a = menu->addAction(tr("Cu&t"), this, SLOT(cut()));
    a->setEnabled(cursor.hasSelection() && editable);

    a = menu->addAction(tr("&Copy"), this, SLOT(copy()));
    a->setEnabled(cursor.hasSelection());

    a = menu->addAction(tr("&Paste"), this, SLOT(paste()));
    a->setEnabled(canPaste() && editable);

    menu->addSeparator();
    a = menu->addAction(tr("Select &All"), this, SLOT(selectAll()));
    a->setEnabled(!document()->isEmpty());

    menu->addSeparator();
    menu->addAction(tr("C&lear"), this, SLOT(clear()));

    menu->exec(event->globalPos());

    delete menu;
}

void QtMessageLogEditor::focusOutEvent(QFocusEvent * /*e*/)
{
    emit editingFinished();
}

void QtMessageLogEditor::handleUpKey()
{
    QTC_ASSERT(m_historyIndex.isValid(), return);
    int currentRow = m_historyIndex.row();
    const QAbstractItemModel *model = m_historyIndex.model();
    if (currentRow == model->rowCount() - 1)
        m_cachedScript = getCurrentScript();

    while (currentRow) {
        currentRow--;
        if (model->hasIndex(currentRow, 0)) {
            QModelIndex index = model->index(currentRow, 0);
            if (QtMessageLogHandler::InputType == (QtMessageLogHandler::ItemType)model->data(
                        index, QtMessageLogHandler::TypeRole).toInt()) {
                m_historyIndex = index;
                replaceCurrentScript(model->data(
                                         index, Qt::DisplayRole).
                                     toString());
                break;
            }
        }
    }
}

void QtMessageLogEditor::handleDownKey()
{
    QTC_ASSERT(m_historyIndex.isValid(), return);
    int currentRow = m_historyIndex.row();
    const QAbstractItemModel *model = m_historyIndex.model();
    while (currentRow < model->rowCount() - 1) {
        currentRow++;
        if (model->hasIndex(currentRow, 0)) {
            QModelIndex index = model->index(currentRow, 0);
            if (QtMessageLogHandler::InputType == (QtMessageLogHandler::ItemType)model->data(
                        index, QtMessageLogHandler::TypeRole).toInt()) {
                m_historyIndex = index;
                if (currentRow == model->rowCount() - 1)
                    replaceCurrentScript(m_cachedScript);
                else
                    replaceCurrentScript(model->data(
                                             index, Qt::DisplayRole).
                                         toString());
                break;
            }
        }
    }
}

QString QtMessageLogEditor::getCurrentScript() const
{
    QTextCursor cursor = textCursor();
    cursor.setPosition(m_startOfEditableArea);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    QString script = cursor.selectedText();
    //remove WS
    return script.trimmed();
}

void QtMessageLogEditor::replaceCurrentScript(const QString &script)
{
    QTextCursor cursor = textCursor();
    cursor.setPosition(m_startOfEditableArea);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    cursor.insertText(script);
    setTextCursor(cursor);
}

} //Internal
} //Debugger
