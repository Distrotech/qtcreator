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

#ifndef KEYWORD_H
#define KEYWORD_H

#include <QColor>
#include <QString>
#include <QList>
#include <QMetaType>

namespace Todo {
namespace Internal {

class Keyword
{
public:
    Keyword();

    QString name;
    QString iconResource;
    QColor color;
    bool equals(const Keyword &other) const;
};

typedef QList<Keyword> KeywordList;

bool operator ==(Keyword &k1, Keyword &k2);
bool operator !=(Keyword &k1, Keyword &k2);

} // namespace Internal
} // namespace Todo

#endif // KEYWORD_H
