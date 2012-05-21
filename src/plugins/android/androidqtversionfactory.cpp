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

#include "androidqtversionfactory.h"
#include "androidqtversion.h"
#include "androidconstants.h"
#include <qtsupport/qtsupportconstants.h>
#include <utils/qtcassert.h>
#include <proparser/profileevaluator.h>

#include <QFileInfo>

namespace Android {
namespace Internal {

AndroidQtVersionFactory::AndroidQtVersionFactory(QObject *parent)
    : QtSupport::QtVersionFactory(parent)
{

}

AndroidQtVersionFactory::~AndroidQtVersionFactory()
{

}

bool AndroidQtVersionFactory::canRestore(const QString &type)
{
    return type == QLatin1String(Constants::ANDROIDQT);
}

QtSupport::BaseQtVersion *AndroidQtVersionFactory::restore(const QString &type,
    const QVariantMap &data)
{
    QTC_ASSERT(canRestore(type), return 0);
    AndroidQtVersion *v = new AndroidQtVersion;
    v->fromMap(data);
    return v;
}

int AndroidQtVersionFactory::priority() const
{
    return 90;
}

QtSupport::BaseQtVersion *AndroidQtVersionFactory::create(const Utils::FileName &qmakePath, ProFileEvaluator *evaluator, bool isAutoDetected, const QString &autoDetectionSource)
{
    QFileInfo fi(qmakePath.toString());
    if (!fi.exists() || !fi.isExecutable() || !fi.isFile())
        return 0;
    if (!evaluator->values(QLatin1String("CONFIG")).contains(QLatin1String("android")))
        return 0;
    return new AndroidQtVersion(qmakePath, isAutoDetected, autoDetectionSource);
}

} // Internal
} // Android
