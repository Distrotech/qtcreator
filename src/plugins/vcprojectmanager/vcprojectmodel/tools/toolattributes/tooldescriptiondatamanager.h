/**************************************************************************
**
** Copyright (c) 2013 Bojan Petrovic
** Copyright (c) 2013 Radovan Zivkvoic
** Contact: http://www.qt-project.org/legal
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/
#ifndef VCPROJECTMANAGER_INTERNAL_TOOLDESCRIPTIONDATAMANAGER_H
#define VCPROJECTMANAGER_INTERNAL_TOOLDESCRIPTIONDATAMANAGER_H

#include <QString>
#include <QHash>
#include <QDomNode>
#include <QDomDocument>
#include <QList>

namespace VcProjectManager {
namespace Internal {

class IAttributeDescriptionDataItem;
class ToolDescription;
class ToolSectionDescription;

struct ToolInfo {
    QString m_key;
    QString m_displayName;

    bool operator==(const ToolInfo &toolInfo) {
        if (m_key != toolInfo.m_key)
            return false;

        return true;
    }

    bool isValid() {
        return !m_key.isEmpty();
    }
};

class ToolDescriptionDataManager : public QObject
{
    friend class VcProjectManagerPlugin;
public:
    static ToolDescriptionDataManager* instance();
    void readToolXMLFiles();
    ~ToolDescriptionDataManager();

    int toolDescriptionCount() const;
    ToolDescription* toolDescription(int index) const;
    ToolDescription* toolDescription(const QString &toolKey) const;

    static ToolInfo readToolInfo(const QString &filePath, QString *errorMsg = 0, int *errorLine = 0, int *errorColumn = 0);

private:
    ToolDescriptionDataManager();
    void readAttributeDataFromFile(const QString &filePath);
    void processXMLDoc(const QDomDocument &xmlDoc);
    void processDomNode(const QDomNode &node);
    void processToolSectionNode(ToolDescription *toolDescription, const QDomNode &domNode);
    void processToolAttributeDescriptions(ToolSectionDescription *toolSectDesc, const QDomNode &domNode);
    ToolDescription* readToolDescription(const QDomNode &domNode);

    QList<ToolDescription *> m_toolDescriptions;
    static ToolDescriptionDataManager *m_instance;
};

} // namespace Internal
} // namespace VcProjectManager

#endif // VCPROJECTMANAGER_INTERNAL_TOOLDESCRIPTIONDATAMANAGER_H
