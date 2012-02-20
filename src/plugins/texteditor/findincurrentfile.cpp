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

#include "findincurrentfile.h"
#include "itexteditor.h"

#include <coreplugin/icore.h>
#include <coreplugin/editormanager/editormanager.h>

#include <QSettings>

using namespace Find;
using namespace TextEditor;
using namespace TextEditor::Internal;

FindInCurrentFile::FindInCurrentFile()
  : m_currentDocument(0)
{
    connect(Core::ICore::editorManager(), SIGNAL(currentEditorChanged(Core::IEditor*)),
            this, SLOT(handleFileChange(Core::IEditor*)));
    handleFileChange(Core::ICore::editorManager()->currentEditor());
}

QString FindInCurrentFile::id() const
{
    return QLatin1String("Current File");
}

QString FindInCurrentFile::displayName() const
{
    return tr("Current File");
}

Utils::FileIterator *FindInCurrentFile::files(const QStringList &nameFilters,
                                              const QVariant &additionalParameters) const
{
    Q_UNUSED(nameFilters)
    QString fileName = additionalParameters.toString();
    QMap<QString, QTextCodec *> openEditorEncodings = ITextEditor::openedTextEditorsEncodings();
    QTextCodec *codec = openEditorEncodings.value(fileName);
    if (!codec)
        codec = Core::EditorManager::instance()->defaultTextCodec();
    return new Utils::FileIterator(QStringList() << fileName, QList<QTextCodec *>() << codec);
}

QVariant FindInCurrentFile::additionalParameters() const
{
    return qVariantFromValue(m_currentDocument->fileName());
}

QString FindInCurrentFile::label() const
{
    return tr("File '%1':").arg(QFileInfo(m_currentDocument->fileName()).fileName());
}

QString FindInCurrentFile::toolTip() const
{
    // %2 is filled by BaseFileFind::runNewSearch
    return tr("File path: %1\n%2").arg(QDir::toNativeSeparators(m_currentDocument->fileName()));
}

bool FindInCurrentFile::isEnabled() const
{
    return m_currentDocument && !m_currentDocument->fileName().isEmpty();
}

void FindInCurrentFile::handleFileChange(Core::IEditor *editor)
{
    if (!editor) {
        if (m_currentDocument) {
            m_currentDocument = 0;
            emit enabledChanged(isEnabled());
        }
    } else {
        Core::IDocument *document = editor->document();
        if (document != m_currentDocument) {
            m_currentDocument = document;
            emit enabledChanged(isEnabled());
        }
    }
}


void FindInCurrentFile::writeSettings(QSettings *settings)
{
    settings->beginGroup(QLatin1String("FindInCurrentFile"));
    writeCommonSettings(settings);
    settings->endGroup();
}

void FindInCurrentFile::readSettings(QSettings *settings)
{
    settings->beginGroup(QLatin1String("FindInCurrentFile"));
    readCommonSettings(settings, QLatin1String("*"));
    settings->endGroup();
}
