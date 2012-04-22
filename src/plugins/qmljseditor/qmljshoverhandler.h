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

#ifndef QMLJSHOVERHANDLER_H
#define QMLJSHOVERHANDLER_H

#include <qmljs/qmljsmodelmanagerinterface.h>
#include <texteditor/basehoverhandler.h>

#include <QColor>

QT_BEGIN_NAMESPACE
template <class> class QList;
QT_END_NAMESPACE

namespace Core {
class IEditor;
}

namespace TextEditor {
class ITextEditor;
}

namespace QmlJS {
class ScopeChain;
class Context;
typedef QSharedPointer<const Context> ContextPtr;
class Value;
}

namespace QmlJSEditor {
class QmlJSTextEditorWidget;

namespace Internal {

class HoverHandler : public TextEditor::BaseHoverHandler
{
    Q_OBJECT
public:
    HoverHandler(QObject *parent = 0);

private:
    void reset();

    virtual bool acceptEditor(Core::IEditor *editor);
    virtual void identifyMatch(TextEditor::ITextEditor *editor, int pos);
    virtual void operateTooltip(TextEditor::ITextEditor *editor, const QPoint &point);

    bool matchDiagnosticMessage(QmlJSEditor::QmlJSTextEditorWidget *qmlEditor, int pos);
    bool matchColorItem(const QmlJS::ScopeChain &lookupContext,
                        const QmlJS::Document::Ptr &qmlDocument,
                        const QList<QmlJS::AST::Node *> &astPath,
                        unsigned pos);
    void handleOrdinaryMatch(const QmlJS::ScopeChain &lookupContext,
                             QmlJS::AST::Node *node);
    void handleImport(const QmlJS::ScopeChain &lookupContext,
                      QmlJS::AST::UiImport *node);

    void prettyPrintTooltip(const QmlJS::Value *value,
                            const QmlJS::ContextPtr &context);

    TextEditor::HelpItem qmlHelpItem(const QmlJS::ScopeChain &lookupContext,
                                     const QmlJS::Document::Ptr &qmlDocument,
                                     QmlJS::AST::Node *node) const;

    QmlJS::ModelManagerInterface *m_modelManager;
    QColor m_colorTip;
};

} // namespace Internal
} // namespace QmlJSEditor

#endif // QMLJSHOVERHANDLER_H
