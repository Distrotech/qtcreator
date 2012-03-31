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

#include "cmakeeditor.h"

#include "cmakehighlighter.h"
#include "cmakeeditorfactory.h"
#include "cmakeprojectconstants.h"
#include "cmakeproject.h"

#include <coreplugin/icore.h>
#include <coreplugin/infobar.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/session.h>
#include <texteditor/fontsettings.h>
#include <texteditor/texteditoractionhandler.h>
#include <texteditor/texteditorconstants.h>
#include <texteditor/texteditorsettings.h>

#include <QFileInfo>

using namespace CMakeProjectManager;
using namespace CMakeProjectManager::Internal;

//
// ProFileEditorEditable
//

CMakeEditor::CMakeEditor(CMakeEditorWidget *editor)
  : BaseTextEditor(editor),
    m_infoBarShown(false)
{
    setContext(Core::Context(CMakeProjectManager::Constants::C_CMAKEEDITOR,
              TextEditor::Constants::C_TEXTEDITOR));
    connect(this, SIGNAL(changed()), this, SLOT(markAsChanged()));
}

Core::IEditor *CMakeEditor::duplicate(QWidget *parent)
{
    CMakeEditorWidget *w = qobject_cast<CMakeEditorWidget*>(widget());
    CMakeEditorWidget *ret = new CMakeEditorWidget(parent, w->factory(), w->actionHandler());
    ret->duplicateFrom(w);
    TextEditor::TextEditorSettings::instance()->initializeEditor(ret);
    return ret->editor();
}

Core::Context CMakeEditor::context() const
{
    return Core::Context(Constants::C_CMAKEEDITOR);
}

Core::Id CMakeEditor::id() const
{
    return CMakeProjectManager::Constants::CMAKE_EDITOR_ID;
}

void CMakeEditor::markAsChanged()
{
    if (!document()->isModified())
        return;
    if (m_infoBarShown)
        return;
    m_infoBarShown = true;
    Core::InfoBarEntry info(QLatin1String("CMakeEditor.RunCMake"),
                            tr("Changes to cmake files are shown in the project tree after building."));
    info.setCustomButtonInfo(tr("Build now"), this, SLOT(build()));
    document()->infoBar()->addInfo(info);
}

void CMakeEditor::build()
{
    QList<ProjectExplorer::Project *> projects =
            ProjectExplorer::ProjectExplorerPlugin::instance()->session()->projects();
    foreach (ProjectExplorer::Project *p, projects) {
        CMakeProject *cmakeProject = qobject_cast<CMakeProject *>(p);
        if (cmakeProject) {
            if (cmakeProject->isProjectFile(document()->fileName())) {
                ProjectExplorer::ProjectExplorerPlugin::instance()->buildProject(cmakeProject);
                break;
            }
        }
    }
}

//
// CMakeEditor
//

CMakeEditorWidget::CMakeEditorWidget(QWidget *parent, CMakeEditorFactory *factory, TextEditor::TextEditorActionHandler *ah)
    : BaseTextEditorWidget(parent), m_factory(factory), m_ah(ah)
{
    CMakeDocument *doc = new CMakeDocument();
    doc->setMimeType(QLatin1String(CMakeProjectManager::Constants::CMAKEMIMETYPE));
    setBaseTextDocument(doc);

    baseTextDocument()->setSyntaxHighlighter(new CMakeHighlighter);

    m_commentDefinition.clearCommentStyles();
    m_commentDefinition.setSingleLine(QLatin1String("#"));

    ah->setupActions(this);
}

TextEditor::BaseTextEditor *CMakeEditorWidget::createEditor()
{
    return new CMakeEditor(this);
}

void CMakeEditorWidget::unCommentSelection()
{
    Utils::unCommentSelection(this, m_commentDefinition);
}

void CMakeEditorWidget::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu *menu = new QMenu();

    Core::ActionManager *am = Core::ICore::instance()->actionManager();
    Core::ActionContainer *mcontext = am->actionContainer(Constants::M_CONTEXT);
    QMenu *contextMenu = mcontext->menu();

    foreach (QAction *action, contextMenu->actions())
        menu->addAction(action);

    appendStandardContextMenuActions(menu);

    menu->exec(e->globalPos());
    delete menu;
}

void CMakeEditorWidget::setFontSettings(const TextEditor::FontSettings &fs)
{
    TextEditor::BaseTextEditorWidget::setFontSettings(fs);
    CMakeHighlighter *highlighter = qobject_cast<CMakeHighlighter*>(baseTextDocument()->syntaxHighlighter());
    if (!highlighter)
        return;

    static QVector<QString> categories;
    if (categories.isEmpty()) {
        categories << QLatin1String(TextEditor::Constants::C_LABEL)  // variables
                << QLatin1String(TextEditor::Constants::C_KEYWORD)   // functions
                << QLatin1String(TextEditor::Constants::C_COMMENT)
                << QLatin1String(TextEditor::Constants::C_STRING)
                << QLatin1String(TextEditor::Constants::C_VISUAL_WHITESPACE);
    }

    const QVector<QTextCharFormat> formats = fs.toTextCharFormats(categories);
    highlighter->setFormats(formats.constBegin(), formats.constEnd());
    highlighter->rehighlight();
}

static bool isValidFileNameChar(const QChar &c)
{
    if (c.isLetterOrNumber()
            || c == QLatin1Char('.')
            || c == QLatin1Char('_')
            || c == QLatin1Char('-')
            || c == QLatin1Char('/')
            || c == QLatin1Char('\\'))
        return true;
    return false;
}

CMakeEditorWidget::Link CMakeEditorWidget::findLinkAt(const QTextCursor &cursor,
                                                      bool/* resolveTarget*/)
{
    Link link;

    int lineNumber = 0, positionInBlock = 0;
    convertPosition(cursor.position(), &lineNumber, &positionInBlock);

    const QString block = cursor.block().text();

    // check if the current position is commented out
    const int hashPos = block.indexOf(QLatin1Char('#'));
    if (hashPos >= 0 && hashPos < positionInBlock)
        return link;

    // find the beginning of a filename
    QString buffer;
    int beginPos = positionInBlock - 1;
    while (beginPos >= 0) {
        QChar c = block.at(beginPos);
        if (isValidFileNameChar(c)) {
            buffer.prepend(c);
            beginPos--;
        } else {
            break;
        }
    }

    // find the end of a filename
    int endPos = positionInBlock;
    while (endPos < block.count()) {
        QChar c = block.at(endPos);
        if (isValidFileNameChar(c)) {
            buffer.append(c);
            endPos++;
        } else {
            break;
        }
    }

    if (buffer.isEmpty())
        return link;

    // TODO: Resolve variables

    QDir dir(QFileInfo(editorDocument()->fileName()).absolutePath());
    QString fileName = dir.filePath(buffer);
    QFileInfo fi(fileName);
    if (fi.exists()) {
        if (fi.isDir()) {
            QDir subDir(fi.absoluteFilePath());
            QString subProject = subDir.filePath("CMakeLists.txt");
            if (QFileInfo(subProject).exists())
                fileName = subProject;
            else
                return link;
        }
        link.fileName = fileName;
        link.begin = cursor.position() - positionInBlock + beginPos + 1;
        link.end = cursor.position() - positionInBlock + endPos;
    }
    return link;
}


//
// CMakeDocument
//

CMakeDocument::CMakeDocument()
    : TextEditor::BaseTextDocument()
{
}

QString CMakeDocument::defaultPath() const
{
    QFileInfo fi(fileName());
    return fi.absolutePath();
}

QString CMakeDocument::suggestedFileName() const
{
    QFileInfo fi(fileName());
    return fi.fileName();
}
