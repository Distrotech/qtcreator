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

#include "qtmessagelogproxymodel.h"

namespace Debugger {
namespace Internal {

QtMessageLogProxyModel::QtMessageLogProxyModel(QObject *parent) :
    QSortFilterProxyModel(parent),
    m_filter(QtMessageLogHandler::DefaultTypes)
{
}

void QtMessageLogProxyModel::setShowLogs(bool show)
{
    m_filter = show ? m_filter | QtMessageLogHandler::DebugType :
                        m_filter & ~QtMessageLogHandler::DebugType;
    setFilterRegExp(QString());
}

void QtMessageLogProxyModel::setShowWarnings(bool show)
{
    m_filter = show ? m_filter | QtMessageLogHandler::WarningType :
                        m_filter & ~QtMessageLogHandler::WarningType;
    setFilterRegExp(QString());
}

void QtMessageLogProxyModel::setShowErrors(bool show)
{
    m_filter = show ? m_filter | QtMessageLogHandler::ErrorType :
                        m_filter & ~QtMessageLogHandler::ErrorType;
    setFilterRegExp(QString());
}

void QtMessageLogProxyModel::selectEditableRow(const QModelIndex &index,
                           QItemSelectionModel::SelectionFlags command)
{
    emit setCurrentIndex(mapFromSource(index), command);
}

bool QtMessageLogProxyModel::filterAcceptsRow(int sourceRow,
         const QModelIndex &sourceParent) const
 {
     QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
     return m_filter.testFlag((QtMessageLogHandler::ItemType)
                              sourceModel()->data(
                                  index, QtMessageLogHandler::TypeRole).toInt());
 }

void QtMessageLogProxyModel::onRowsInserted(const QModelIndex &index, int start, int end)
{
    int rowIndex = end;
    do {
        if (filterAcceptsRow(rowIndex, index)) {
            emit scrollToBottom();
            break;
        }
    } while (--rowIndex >= start);
}

} // namespace Internal
} // namespace Debugger
