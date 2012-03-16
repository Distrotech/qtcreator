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

#ifndef COMMENTSSETTINGS_H
#define COMMENTSSETTINGS_H

#include "cpptools_global.h"

QT_BEGIN_NAMESPACE
class QSettings;
QT_END_NAMESPACE

namespace CppTools {

class CPPTOOLS_EXPORT CommentsSettings
{
public:
    CommentsSettings();

    void toSettings(const QString &category, QSettings *s) const;
    void fromSettings(const QString &category, QSettings *s);

    bool equals(const CommentsSettings &other) const;

    bool m_enableDoxygen;
    bool m_generateBrief;
    bool m_leadingAsterisks;
};

inline bool operator==(const CommentsSettings &a, const CommentsSettings &b)
{ return a.equals(b); }

inline bool operator!=(const CommentsSettings &a, const CommentsSettings &b)
{ return !(a == b); }

} // namespace CppTools

#endif // COMMENTSSETTINGS_H
