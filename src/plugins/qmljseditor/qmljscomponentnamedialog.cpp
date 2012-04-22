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

#include "qmljscomponentnamedialog.h"
#include "ui_qmljscomponentnamedialog.h"

#include <QFileInfo>
#include <QFileDialog>
#include <QPushButton>

using namespace QmlJSEditor::Internal;

ComponentNameDialog::ComponentNameDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ComponentNameDialog)
{
    ui->setupUi(this);

    connect(ui->pathEdit, SIGNAL(changed(QString)),
            this, SLOT(validate()));
    connect(ui->componentNameEdit, SIGNAL(textChanged(QString)),
            this, SLOT(validate()));
}

ComponentNameDialog::~ComponentNameDialog()
{
    delete ui;
}

void ComponentNameDialog::go(QString *proposedName,
                             QString *proposedPath,
                             QWidget *parent)
{
    Q_ASSERT(proposedName);
    Q_ASSERT(proposedPath);

    ComponentNameDialog d(parent);
    d.ui->componentNameEdit->setNamespacesEnabled(false);
    d.ui->componentNameEdit->setLowerCaseFileName(false);
    d.ui->componentNameEdit->setForceFirstCapitalLetter(true);
    d.ui->componentNameEdit->setText(*proposedName);
    d.ui->pathEdit->setExpectedKind(Utils::PathChooser::ExistingDirectory);
    d.ui->pathEdit->setPath(*proposedPath);

    if (QDialog::Accepted == d.exec()) {
        *proposedName = d.ui->componentNameEdit->text();
        *proposedPath = d.ui->pathEdit->path();
    }
}

void ComponentNameDialog::choosePath()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Choose a path"),
                                                    ui->pathEdit->path());
    if (!dir.isEmpty())
        ui->pathEdit->setPath(dir);
}

void ComponentNameDialog::validate()
{
    const QString message = isValid();
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(message.isEmpty());
    ui->messageLabel->setText(message);
}

QString ComponentNameDialog::isValid() const
{
    if (!ui->componentNameEdit->isValid())
        return ui->componentNameEdit->errorMessage();

    QString compName = ui->componentNameEdit->text();
    if (compName.isEmpty() || !compName[0].isUpper())
        return tr("Invalid component name");

    if (!ui->pathEdit->isValid())
        return tr("Invalid path");

    return QString();
}
