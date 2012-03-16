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

#ifndef TODOITEMSPROVIDER_H
#define TODOITEMSPROVIDER_H

#include "todoitem.h"
#include "settings.h"

#include <projectexplorer/project.h>
#include <coreplugin/editormanager/ieditor.h>

#include <QHash>
#include <QList>

namespace Todo {
namespace Internal {

class TodoItemsModel;
class TodoItemsScanner;

class TodoItemsProvider : public QObject
{
    Q_OBJECT

public:
    explicit TodoItemsProvider(Settings settings, QObject *parent = 0);
    TodoItemsModel *todoItemsModel();

public slots:
    void settingsChanged(const Settings &newSettings);

signals:
    void itemsUpdated();

private:
    Settings m_settings;
    TodoItemsModel *m_itemsModel;

    // All to-do items are stored here regardless current scanning scope
    QHash<QString, QList<TodoItem> > m_itemsHash;

    // This list contains only those to-do items that are within current scanning scope
    QList<TodoItem> m_itemsList;

    QList<TodoItemsScanner *> m_scanners;

    ProjectExplorer::Project *m_startupProject;
    Core::IEditor* m_currentEditor;

    bool m_shouldUpdateList;

    void setupItemsModel();
    void setupStartupProjectBinding();
    void setupCurrentEditorBinding();
    void setupUpdateListTimer();
    void updateList();
    void createScanners();
    void setItemsListWithinStartupProject();

private slots:
    void itemsFetched(const QString &fileName, const QList<TodoItem> &items);
    void startupProjectChanged(ProjectExplorer::Project *project);
    void projectsFilesChanged();
    void currentEditorChanged(Core::IEditor *editor);
    void updateListTimeoutElapsed();
};

}
}

#endif // TODOITEMSPROVIDER_H
