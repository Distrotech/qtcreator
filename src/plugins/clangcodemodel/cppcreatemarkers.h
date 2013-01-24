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

#ifndef CPPCREATEMARKERS_H
#define CPPCREATEMARKERS_H

#include "fastindexer.h"
#include "sourcemarker.h"
#include "semanticmarker.h"

#include <texteditor/semantichighlighter.h>

#include <QSet>
#include <QFuture>
#include <QtConcurrentRun>

namespace ClangCodeModel {

class CreateMarkers:
        public QObject,
        public QRunnable,
        public QFutureInterface<TextEditor::SemanticHighlighter::Result>
{
    Q_OBJECT

public:
    virtual ~CreateMarkers();

    virtual void run();

    typedef TextEditor::SemanticHighlighter::Result SourceMarker;

    typedef QFuture<SourceMarker> Future;

    Future start()
    {
        this->setRunnable(this);
        this->reportStarted();
        Future future = this->future();
        QThreadPool::globalInstance()->start(this, QThread::LowestPriority);
        return future;
    }

    static CreateMarkers *create(ClangCodeModel::SemanticMarker::Ptr semanticMarker,
                                 const QString &fileName,
                                 const QStringList &options,
                                 unsigned firstLine, unsigned lastLine,
                                 Internal::FastIndexer *fastIndexer);

    void addUse(const SourceMarker &marker);
    void flush();

signals:
    void diagnosticsReady(const QList<ClangCodeModel::Diagnostic> &diagnostics);

protected:
    CreateMarkers(ClangCodeModel::SemanticMarker::Ptr semanticMarker,
                  const QString &fileName, const QStringList &options,
                  unsigned firstLine, unsigned lastLine,
                  Internal::FastIndexer *fastIndexer);

private:
    ClangCodeModel::SemanticMarker::Ptr m_marker;
    QString m_fileName;
    QStringList m_options;
    unsigned m_firstLine;
    unsigned m_lastLine;
    Internal::FastIndexer *m_fastIndexer;
    QVector<SourceMarker> m_usages;
    bool m_flushRequested;
    unsigned m_flushLine;

    ClangCodeModel::Internal::UnsavedFiles m_unsavedFiles;
};

} // namespace ClangCodeModel

#endif // CPPCREATEMARKERS_H
