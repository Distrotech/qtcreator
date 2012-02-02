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

#include <QtTest>
#include <QtDebug>

#include <utils/fileutils.h>

//TESTED_COMPONENT=src/libs/utils
using namespace Utils;

class tst_fileutils : public QObject
{
    Q_OBJECT

public:

private slots:
    void parentDir_data();
    void parentDir();
};

void tst_fileutils::parentDir_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<QString>("parentPath");
    QTest::addColumn<QString>("expectFailMessage");

    QTest::newRow("empty path") << QString() << QString() << QString();
    QTest::newRow("root only") << QString::fromLatin1("/") << QString() << QString();
    QTest::newRow("//") << QString::fromLatin1("//") << QString() << QString();
    QTest::newRow("/tmp/dir") << QString::fromLatin1("/tmp/dir") << QString::fromLatin1("/tmp") << QString();
    QTest::newRow("relative/path") << QString::fromLatin1("relative/path") << QString::fromLatin1("relative") << QString();
    QTest::newRow("relativepath") << QString::fromLatin1("relativepath") << QString::fromLatin1(".")
                                  << QString::fromLatin1("see QTBUG-23892");

    // Windows stuff:
#ifdef Q_OS_WIN
    QTest::newRow("C:/data") << QString::fromLatin1("C:/data") << QString::fromLatin1("C:/") << QString();
    QTest::newRow("C:/") << QString::fromLatin1("C:/") << QString() << QString();
    QTest::newRow("//./com1") << QString::fromLatin1("//./com1") << QString::fromLatin1("/") << QString();
    QTest::newRow("//?/path") << QString::fromLatin1("//?/path") << QString::fromLatin1("/")
                              << QString::fromLatin1("Qt 4 can not handle this path.");
    QTest::newRow("/Global??/UNC/host") << QString::fromLatin1("/Global??/UNC/host")
                                        << QString::fromLatin1("/Global??/UNC/host")
                                        << QString::fromLatin1("Qt 4 can not handle this path.");
    QTest::newRow("//server/directory/file")
            << QString::fromLatin1("//server/directory/file") << QString::fromLatin1("//server/directory") << QString();
    QTest::newRow("//server/directory")
            << QString::fromLatin1("//server/directory") << QString::fromLatin1("//server") << QString();
    QTest::newRow("//server")
            << QString::fromLatin1("//server") << QString() << QString();
#endif
}

void tst_fileutils::parentDir()
{
    QFETCH(QString, path);
    QFETCH(QString, parentPath);
    QFETCH(QString, expectFailMessage);

    FileName result = FileName::fromString(path).parentDir();
    if (!expectFailMessage.isEmpty())
        QEXPECT_FAIL("", expectFailMessage.toUtf8().constData(), Continue);
    QCOMPARE(result.toString(), parentPath);
}

QTEST_APPLESS_MAIN(tst_fileutils)
#include "tst_fileutils.moc"
