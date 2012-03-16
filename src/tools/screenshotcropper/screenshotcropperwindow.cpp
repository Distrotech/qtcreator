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

#include "screenshotcropperwindow.h"
#include "ui_screenshotcropperwindow.h"
#include <QListWidget>
#include <QDebug>

using namespace QtSupport::Internal;

ScreenShotCropperWindow::ScreenShotCropperWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ScreenShotCropperWindow)
{
    ui->setupUi(this);
    connect(ui->m_filenamesList, SIGNAL(currentRowChanged(int)), SLOT(selectImage(int)));
    connect(ui->m_cropImageView, SIGNAL(cropAreaChanged(QRect)), SLOT(setArea(QRect)));
    connect(ui->m_buttonBox, SIGNAL(accepted()), SLOT(saveData()));
    connect(ui->m_buttonBox, SIGNAL(rejected()), SLOT(close()));
}

ScreenShotCropperWindow::~ScreenShotCropperWindow()
{
    delete ui;
}

void ScreenShotCropperWindow::loadData(const QString &areasXmlFile, const QString &imagesFolder)
{
    m_areasOfInterestFile = areasXmlFile;
    m_areasOfInterest = ScreenshotCropper::loadAreasOfInterest(m_areasOfInterestFile);
    m_imagesFolder = imagesFolder;
    foreach (const QString &pngFile, m_areasOfInterest.keys())
        ui->m_filenamesList->addItem(pngFile);
}

void ScreenShotCropperWindow::selectImage(int index)
{
    const QString fileName = ui->m_filenamesList->item(index)->text();
    ui->m_cropImageView->setImage(QImage(m_imagesFolder + QLatin1Char('/') + fileName));
    ui->m_cropImageView->setArea(m_areasOfInterest.value(fileName));
}

void ScreenShotCropperWindow::setArea(const QRect &area)
{
    const QListWidgetItem *item = ui->m_filenamesList->currentItem();
    if (!item)
        return;
    const QString currentFile = item->text();
    m_areasOfInterest.insert(currentFile, area);
}

void ScreenShotCropperWindow::saveData()
{
    if (!ScreenshotCropper::saveAreasOfInterest(m_areasOfInterestFile, m_areasOfInterest))
        qFatal("Cannot write %s", qPrintable(m_areasOfInterestFile));
}
