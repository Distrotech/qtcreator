/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2012 Dmitry Savchenko.
** Copyright (c) 2010 Vasiliy Sorokin.
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

#include "todoitemsmodel.h"
#include "constants.h"

#include <QIcon>

namespace Todo {
namespace Internal {

TodoItemsModel::TodoItemsModel(QObject *parent) :
    QAbstractTableModel(parent),
    m_todoItemsList(0),
    m_currentSortColumn(Constants::OutputColumnIndex(0)),
    m_currentSortOrder(Qt::AscendingOrder)
{
}

void TodoItemsModel::setTodoItemsList(QList<TodoItem> *list)
{
    m_todoItemsList = list;
    todoItemsListUpdated();
}

int TodoItemsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    // There's only one level of hierarchy
    if (parent.isValid())
        return 0;

    if (!m_todoItemsList)
        return 0;

    return m_todoItemsList->count();
}

int TodoItemsModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return Constants::OUTPUT_COLUMN_LAST;
}

QVariant TodoItemsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    TodoItem item = m_todoItemsList->at(index.row());

    if (role == Qt::BackgroundColorRole)
        return item.color;

    switch (index.column()) {

        case Constants::OUTPUT_COLUMN_TEXT:
            switch (role) {
                case Qt::DisplayRole:
                    return item.text;
                case Qt::DecorationRole:
                    return QVariant::fromValue(QIcon(item.iconResource));
            }
            break;

        case Constants::OUTPUT_COLUMN_FILE:
            if (role == Qt::DisplayRole)
                return item.file;
            break;

        case Constants::OUTPUT_COLUMN_LINE:
            if (role == Qt::DisplayRole)
                return item.line;
            break;
    }

    return QVariant();
}

QVariant TodoItemsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical)
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    switch (section) {
        case Constants::OUTPUT_COLUMN_TEXT:
            return tr(Constants::OUTPUT_COLUMN_TEXT_TITLE);

        case Constants::OUTPUT_COLUMN_FILE:
            return tr(Constants::OUTPUT_COLUMN_FILE_TITLE);

        case Constants::OUTPUT_COLUMN_LINE:
            return tr(Constants::OUTPUT_COLUMN_LINE_TITLE);

        default:
            return QVariant();
    }
}

void TodoItemsModel::sort(int column, Qt::SortOrder order)
{
    m_currentSortColumn = Constants::OutputColumnIndex(column);
    m_currentSortOrder = order;

    TodoItemSortPredicate predicate(m_currentSortColumn, m_currentSortOrder);
    qSort(m_todoItemsList->begin(), m_todoItemsList->end(), predicate);
    emit layoutChanged();
}

void TodoItemsModel::todoItemsListUpdated()
{
    if (!m_todoItemsList)
        return;

    sort(m_currentSortColumn, m_currentSortOrder);
}

}
}
