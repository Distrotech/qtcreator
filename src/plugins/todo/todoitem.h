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

#ifndef TODOITEM_H
#define TODOITEM_H

#include "constants.h"

#include <QMetaType>
#include <QString>
#include <QColor>

namespace Todo {
namespace Internal {

class TodoItem
{
public:
    TodoItem() : line(-1) {}

    QString text;
    QString file;
    int line;
    QString iconResource;
    QColor color;
};

class TodoItemSortPredicate
{
public:
    explicit TodoItemSortPredicate(Constants::OutputColumnIndex columnIndex, Qt::SortOrder order) :
        m_columnIndex(columnIndex),
        m_order(order)
    {}

    inline bool operator()(const TodoItem &t1, const TodoItem &t2)
    {
        if (m_order == Qt::AscendingOrder)
            return lessThan(t1, t2);
        else
            return lessThan(t2, t1);
    }

    inline bool lessThan(const TodoItem &t1, const TodoItem &t2)
    {
        switch (m_columnIndex) {
            case Constants::OUTPUT_COLUMN_TEXT:
                return t1.text < t2.text;

            case Constants::OUTPUT_COLUMN_LINE:
                return t1.line < t2.line;

            case Constants::OUTPUT_COLUMN_FILE:
                return t1.file < t2.file;

            default:
                Q_ASSERT(false);
                return false;
        }
    }

private:
    Constants::OutputColumnIndex m_columnIndex;
    Qt::SortOrder m_order;
};

} // namespace Internal
} // namespace Todo

Q_DECLARE_METATYPE(Todo::Internal::TodoItem)

#endif // TODOITEM_H
