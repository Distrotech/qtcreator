/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2012 BogDan Vatra <bog_dan_ro@yahoo.com>
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

#ifndef JAVAPARSER_H
#define JAVAPARSER_H
#include <projectexplorer/ioutputparser.h>

namespace Android {
namespace Internal {

class JavaParser : public ProjectExplorer::IOutputParser
{
    Q_OBJECT
public:
    JavaParser();
    virtual void stdOutput(const QString &line);
    virtual void stdError(const QString &line);
    void setProjectFileList(const QStringList &fileList);
private:
    QRegExp m_javaRegExp;
    QStringList m_fileList;
};

} // namespace Internal
} // namespace Android

#endif // JAVAPARSER_H
