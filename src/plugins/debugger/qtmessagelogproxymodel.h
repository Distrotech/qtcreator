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

#ifndef QTMESSAGELOGPROXYMODEL_H
#define QTMESSAGELOGPROXYMODEL_H

#include "qtmessageloghandler.h"

#include <QSortFilterProxyModel>
#include <QItemSelectionModel>

namespace Debugger {
namespace Internal {

class QtMessageLogProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit QtMessageLogProxyModel(QObject *parent = 0);

public slots:
    void setShowLogs(bool show);
    void setShowWarnings(bool show);
    void setShowErrors(bool show);
    void selectEditableRow(const QModelIndex &index,
                               QItemSelectionModel::SelectionFlags command);
    void onRowsInserted(const QModelIndex &index, int start, int end);

signals:
    void scrollToBottom();
    void setCurrentIndex(const QModelIndex &index,
                         QItemSelectionModel::SelectionFlags command);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;

private:
    QFlags<QtMessageLogHandler::ItemType> m_filter;
};

} // namespace Internal
} // namespace Debugger

#endif // QTMESSAGELOGPROXYMODEL_H
