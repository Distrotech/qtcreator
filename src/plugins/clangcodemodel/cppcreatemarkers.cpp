/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2011 Nokia Corporation and/or its subsidiary(-ies).
**
** Contact: Nokia Corporation (info@qt.nokia.com)
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

#include "clangutils.h"
#include "cppcreatemarkers.h"

#include <utils/runextensions.h>

#include <QCoreApplication>
#include <QMutexLocker>
#include <QThreadPool>

#include <QDebug>

//#define DEBUG_TIMING

using namespace ClangCodeModel;
using namespace ClangCodeModel::Internal;
using namespace CppTools;

CreateMarkers *CreateMarkers::create(SemanticMarker::Ptr semanticMarker,
                                     const QString &fileName,
                                     const QStringList &options,
                                     unsigned firstLine, unsigned lastLine,
                                     FastIndexer *fastIndexer)
{
    if (semanticMarker.isNull())
        return 0;
    else
        return new CreateMarkers(semanticMarker, fileName, options, firstLine, lastLine, fastIndexer);
}

CreateMarkers::CreateMarkers(SemanticMarker::Ptr semanticMarker,
                             const QString &fileName,
                             const QStringList &options,
                             unsigned firstLine, unsigned lastLine,
                             FastIndexer *fastIndexer)
    : m_marker(semanticMarker)
    , m_fileName(fileName)
    , m_options(options)
    , m_firstLine(firstLine)
    , m_lastLine(lastLine)
    , m_fastIndexer(fastIndexer)
{
    Q_ASSERT(!semanticMarker.isNull());

    m_flushRequested = false;
    m_flushLine = 0;

    m_unsavedFiles = Utils::createUnsavedFiles(CPlusPlus::CppModelManagerInterface::instance()->workingCopy());
}

CreateMarkers::~CreateMarkers()
{ }

void CreateMarkers::run()
{
    QMutexLocker lock(m_marker->mutex());
    if (isCanceled())
        return;

#ifdef DEBUG_TIMING
    qDebug() << "*** Highlighting from" << m_firstLine << "to" << m_lastLine << "of" << m_fileName;
    qDebug() << "***** Options: " << m_options.join(" ");
    QTime t; t.start();
#endif // DEBUG_TIMING

    m_usages.clear();
    m_marker->setFileName(m_fileName);
    m_marker->setCompilationOptions(m_options);

    m_marker->reparse(m_unsavedFiles);
#ifdef DEBUG_TIMING
    qDebug() << "*** Reparse for highlighting took" << t.elapsed() << "ms.";
#endif // DEBUG_TIMING

    QList<ClangCodeModel::Diagnostic> diagnostics;
    foreach (const ClangCodeModel::Diagnostic &d, m_marker->diagnostics()) {
#ifdef DEBUG_TIMING
        qDebug() << d.severityAsString() << d.location() << d.spelling();
#endif // DEBUG_TIMING
        if (d.location().fileName() == m_marker->fileName())
            diagnostics.append(d);
    }
    emit diagnosticsReady(diagnostics);

    if (isCanceled())
        return;

    QList<ClangCodeModel::SourceMarker> markers = m_marker->sourceMarkersInRange(m_firstLine, m_lastLine);
    foreach (const ClangCodeModel::SourceMarker &m, markers)
        addUse(SourceMarker(m.location().line(), m.location().column(), m.length(), m.kind()));

    if (isCanceled())
        return;

    flush();
    reportFinished();

#ifdef DEBUG_TIMING
    qDebug() << "*** Highlighting took" << t.elapsed() << "ms in total.";
    t.restart();
#endif // DEBUG_TIMING

    if (m_fastIndexer)
        m_fastIndexer->indexNow(m_marker->unit());

#ifdef DEBUG_TIMING
    qDebug() << "*** Fast re-indexing took" << t.elapsed() << "ms in total.";
#endif // DEBUG_TIMING
}

void CreateMarkers::addUse(const SourceMarker &marker)
{
//    if (! enclosingFunctionDefinition()) {
        if (m_usages.size() >= 100) {
            if (m_flushRequested && marker.line != m_flushLine)
                flush();
            else if (! m_flushRequested) {
                m_flushRequested = true;
                m_flushLine = marker.line;
            }
        }
//    }

    m_usages.append(marker);
}

void CreateMarkers::flush()
{
    m_flushRequested = false;
    m_flushLine = 0;

    if (m_usages.isEmpty())
        return;

    reportResults(m_usages);
    m_usages.clear();
}
