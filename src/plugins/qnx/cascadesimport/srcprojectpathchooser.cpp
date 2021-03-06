/****************************************************************************
**
** Copyright (C) 2015 BlackBerry Limited. All rights reserved.
**
** Contact: BlackBerry (qt@blackberry.com)
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company.  For licensing terms and
** conditions see http://www.qt.io/terms-conditions.  For further information
** use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file.  Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, The Qt Company gives you certain additional
** rights.  These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#include "srcprojectpathchooser.h"

#include <QDirIterator>

namespace Qnx {
namespace Internal {

SrcProjectPathChooser::SrcProjectPathChooser(QWidget *parent) :
    Utils::PathChooser(parent)
{
    setPromptDialogTitle(tr("Choose imported Cascades project directory"));
    setExpectedKind(Utils::PathChooser::ExistingDirectory);
}

SrcProjectPathChooser::~SrcProjectPathChooser()
{
}

bool SrcProjectPathChooser::validatePath(const QString &path, QString *errorMessage)
{
    if (!Utils::PathChooser::validatePath(path, errorMessage))
        return false;

    bool proFound = false;
    bool barDescriptorFound = false;
    QDirIterator di(path);
    while (di.hasNext()) {
        di.next();
        QFileInfo fi = di.fileInfo();
        if (fi.isFile()) {
            if (fi.fileName() == QLatin1String("bar-descriptor.xml"))
                barDescriptorFound = true;
            else if (fi.fileName().endsWith(QLatin1String(".pro")))
                proFound = true;
        }
        if (barDescriptorFound && proFound)
            break;
    }
    const bool ret = barDescriptorFound && proFound;
    if (!ret && errorMessage)
        *errorMessage = tr("Directory does not seem to be a valid Cascades project.");
    return ret;
}


} // namespace Internal
} // namespace Qnx
