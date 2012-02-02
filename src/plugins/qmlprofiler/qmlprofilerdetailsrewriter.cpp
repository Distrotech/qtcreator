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

#include "qmlprofilerdetailsrewriter.h"

#include <qmljs/parser/qmljsast_p.h>
#include <qmljs/qmljsmodelmanagerinterface.h>
#include <qmljstools/qmljsmodelmanager.h>

#include <utils/qtcassert.h>

namespace QmlProfiler {
namespace Internal {

struct PendingEvent {
    QmlJsDebugClient::QmlEventLocation location;
    QString localFile;
    int eventType;
};

class PropertyVisitor: protected QmlJS::AST::Visitor
{
    QmlJS::AST::Node * _lastValidNode;
    unsigned _line;
    unsigned _col;
public:
    QmlJS::AST::Node * operator()(QmlJS::AST::Node *node, unsigned line, unsigned col)
    {
        _line = line;
        _col = col;
        _lastValidNode = 0;
        accept(node);
        return _lastValidNode;
    }

protected:
    using QmlJS::AST::Visitor::visit;

    void accept(QmlJS::AST::Node *node)
    {
        if (node)
            node->accept(this);
    }

    bool containsLocation(QmlJS::AST::SourceLocation start, QmlJS::AST::SourceLocation end)
    {
        return (_line > start.startLine || (_line == start.startLine && _col >= start.startColumn)) &&
                (_line < end.startLine || (_line == end.startLine && _col <= end.startColumn));
    }


    virtual bool preVisit(QmlJS::AST::Node *node)
    {
        if (QmlJS::AST::cast<QmlJS::AST::UiQualifiedId *>(node)) {
            return false;
        }
        return containsLocation(node->firstSourceLocation(), node->lastSourceLocation());
    }

    virtual bool visit(QmlJS::AST::UiScriptBinding *ast)
    {
        _lastValidNode = ast;
        return true;
    }

    virtual bool visit(QmlJS::AST::UiPublicMember *ast)
    {
        _lastValidNode = ast;
        return true;
    }
};

/////////////////////////////////////////////////////////////////////////////////

class QmlProfilerDetailsRewriter::QmlProfilerDetailsRewriterPrivate
{
public:
    QmlProfilerDetailsRewriterPrivate(QmlProfilerDetailsRewriter *qq) : q(qq) {}
    ~QmlProfilerDetailsRewriterPrivate() {}

    QList <PendingEvent> m_pendingEvents;
    QStringList m_pendingDocs;

    QmlProfilerDetailsRewriter *q;
};

QmlProfilerDetailsRewriter::QmlProfilerDetailsRewriter(QObject *parent) : QObject(parent),
    d(new QmlProfilerDetailsRewriterPrivate(this))
{ }

QmlProfilerDetailsRewriter::~QmlProfilerDetailsRewriter()
{
    delete d;
}

void QmlProfilerDetailsRewriter::requestDetailsForLocation(int type,
        const QmlJsDebugClient::QmlEventLocation &location)
{
    QString localFile = QUrl(location.filename).toLocalFile();

    if (QmlJSTools::languageOfFile(localFile) != QmlJS::Document::QmlLanguage)
        return;

    PendingEvent ev = {location, localFile, type};
    d->m_pendingEvents << ev;
    if (!d->m_pendingDocs.contains(localFile)) {
        if (d->m_pendingDocs.isEmpty())
            connect(QmlJS::ModelManagerInterface::instance(),
                    SIGNAL(documentUpdated(QmlJS::Document::Ptr)),
                    this,
                    SLOT(documentReady(QmlJS::Document::Ptr)));

        d->m_pendingDocs << localFile;
    }
}

void QmlProfilerDetailsRewriter::reloadDocuments()
{
    if (!d->m_pendingDocs.isEmpty())
        QmlJS::ModelManagerInterface::instance()->updateSourceFiles(d->m_pendingDocs, false);
    else
        emit eventDetailsChanged();
}

void QmlProfilerDetailsRewriter::rewriteDetailsForLocation(QTextStream &textDoc,
        QmlJS::Document::Ptr doc, int type, const QmlJsDebugClient::QmlEventLocation &location)
{
    PropertyVisitor propertyVisitor;
    QmlJS::AST::Node *node = propertyVisitor(doc->ast(), location.line, location.column);

    QTC_ASSERT(node, return);

    qint64 startPos = node->firstSourceLocation().begin();
    qint64 len = node->lastSourceLocation().end() - startPos;

    textDoc.seek(startPos);
    QString details = textDoc.read(len).replace('\n'," ").simplified();

    emit rewriteDetailsString(type, location, details);
}

void QmlProfilerDetailsRewriter::documentReady(QmlJS::Document::Ptr doc)
{
    // this could be triggered by an unrelated reload in Creator
    if (!d->m_pendingDocs.contains(doc->fileName()))
        return;

    // if the file could not be opened this slot is still triggered but source will be an empty string
    QString source = doc->source();
    if (!source.isEmpty()) {
        QTextStream st(&source, QIODevice::ReadOnly);

        for (int i = d->m_pendingEvents.count()-1; i>=0; i--) {
            PendingEvent ev = d->m_pendingEvents[i];
            if (ev.localFile == doc->fileName()) {
                d->m_pendingEvents.removeAt(i);
                rewriteDetailsForLocation(st, doc, ev.eventType, ev.location);
            }
        }
    }

    d->m_pendingDocs.removeOne(doc->fileName());

    if (d->m_pendingDocs.isEmpty()) {
        disconnect(QmlJS::ModelManagerInterface::instance(),
                   SIGNAL(documentUpdated(QmlJS::Document::Ptr)),
                   this,
                   SLOT(documentReady(QmlJS::Document::Ptr)));
        emit eventDetailsChanged();
    }
}

} // namespace Internal
} // namespace QmlProfiler
