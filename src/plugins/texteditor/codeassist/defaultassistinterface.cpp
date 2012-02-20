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

#include "defaultassistinterface.h"

#include <texteditor/convenience.h>

#include <QThread>
#include <QTextDocument>
#include <QTextCursor>

using namespace TextEditor;

DefaultAssistInterface::DefaultAssistInterface(QTextDocument *textDocument,
                                               int position,
                                               Core::IDocument *document,
                                               AssistReason reason)
    : m_textDocument(textDocument)
    , m_detached(false)
    , m_position(position)
    , m_document(document)
    , m_reason(reason)
{}

DefaultAssistInterface::~DefaultAssistInterface()
{
    if (m_detached)
        delete m_textDocument;
}

QChar DefaultAssistInterface::characterAt(int position) const
{
    return m_textDocument->characterAt(position);
}

QString DefaultAssistInterface::textAt(int pos, int length) const
{
    return Convenience::textAt(QTextCursor(m_textDocument), pos, length);
}

void DefaultAssistInterface::detach(QThread *destination)
{
    m_textDocument = m_textDocument->clone();
    m_textDocument->moveToThread(destination);
    m_detached = true;
}

AssistReason DefaultAssistInterface::reason() const
{
    return m_reason;
}
