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

#ifndef FEATUREMANAGER_H
#define FEATUREMANAGER_H

#include "core_global.h"

#include <coreplugin/id.h>

#include <QObject>
#include <QSet>
#include <QStringList>


namespace Utils {
class AbstractMacroExpander;
}

namespace Core {

class CORE_EXPORT FeatureSet;

class CORE_EXPORT IFeatureProvider : public QObject
{
    Q_OBJECT

public:
    IFeatureProvider() {}
    virtual ~IFeatureProvider() {}
    virtual FeatureSet availableFeatures(const QString &platform) const = 0;
    virtual QStringList availablePlatforms() const = 0;
    virtual QString displayNameForPlatform(const QString &string) const = 0;
};

class CORE_EXPORT Feature : public Id
{
friend class FeatureSet;
public:
    Feature(const char *name) : Id(name) {}
    explicit Feature(const QString &name) : Id(name) {}
};

class CORE_EXPORT FeatureSet : private QSet<Feature>
{
public:
    FeatureSet() {}

    FeatureSet(const Feature &feature)
    {
        if (feature.toString().isEmpty())
            return;

        insert(feature);
    }

    FeatureSet(const FeatureSet &other) : QSet<Feature>(other) {}

    FeatureSet &operator=(const FeatureSet &other)
    {
       QSet<Feature>::operator=(other);
       return *this;
    }

    bool contains(const Feature &feature) const
    {
        return QSet<Feature>::contains(feature);
    }

    bool contains(const FeatureSet &features) const
    {
        return QSet<Feature>::contains(features);
    }

    void remove(const Feature &feature)
    {
        QSet<Feature>::remove(feature);
    }

    FeatureSet operator|(const Feature &feature) const
    {
        FeatureSet copy = *this;
        if (feature.isValid())
            copy.insert(feature);
        return copy;
    }

    FeatureSet operator|(const FeatureSet &features) const
    {
        FeatureSet copy = *this;
        if (!features.isEmpty())
            copy.unite(features);
        return copy;
    }

    FeatureSet &operator|=(const Feature &feature)
    {
        if (feature.isValid())
           insert(feature);
        return *this;
    }

    FeatureSet &operator|=(const FeatureSet &features)
    {
        if (!features.isEmpty())
            unite(features);
        return *this;
    }

    QStringList toStringList() const
    {
        QStringList stringList;
        foreach (const Feature &feature, QSet<Feature>(*this))
            stringList.append(feature.toString());
        return stringList;
    }
};

} // namespace Core

/*
The following operators have to be defined in the global namespace!
Otherwise "using namespace Core" would hide other | operators
defined in the global namespace (e. g. QFlags).
*/

inline Core::FeatureSet operator |(Core::Feature feature1, Core::Feature feature2)
{ return Core::FeatureSet(feature1) | feature2; }

inline Core::FeatureSet operator|(Core::Feature feature1, Core::FeatureSet feature2)
{ return feature2 | feature1; }


#endif // FEATUREANAGER_H
