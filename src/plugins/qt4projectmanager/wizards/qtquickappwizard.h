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

#ifndef QTQUICKAPPWIZARD_H
#define QTQUICKAPPWIZARD_H

#include "abstractmobileappwizard.h"

namespace Qt4ProjectManager {
namespace Internal {

class QtQuickAppWizard : public AbstractMobileAppWizard
{
    Q_OBJECT
public:

    enum Kind {
        QtQuick1_1 = 0,
        SymbianComponents = 1,
        MeegoComponents = 2,
        ImportQml = 3
    };

    QtQuickAppWizard();
    explicit QtQuickAppWizard(const Core::BaseFileWizardParameters &params, QObject *parent = 0);
    virtual ~QtQuickAppWizard();
    static void createInstances(ExtensionSystem::IPlugin *plugin);

protected:
    QString fileToOpenPostGeneration() const;

private:
    static Core::BaseFileWizardParameters baseParameters();

    virtual AbstractMobileApp *app() const;
    virtual AbstractMobileAppWizardDialog *wizardDialog() const;
    virtual AbstractMobileAppWizardDialog *createWizardDialogInternal(QWidget *parent,
                                                                      const Core::WizardDialogParameters &baseParameters) const;
    virtual void projectPathChanged(const QString &path) const;
    virtual void prepareGenerateFiles(const QWizard *wizard,
        QString *errorMessage) const;
    void setQtQuickKind(Kind kind);
    Kind qtQuickKind() const;

    class QtQuickAppWizardPrivate *d;
};

} // namespace Internal
} // namespace Qt4ProjectManager

#endif // QTQUICKAPPWIZARD_H
