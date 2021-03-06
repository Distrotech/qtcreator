/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing
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

#ifndef JSONWIZARD_H
#define JSONWIZARD_H

#include "../projectexplorer_export.h"

#include <coreplugin/generatedfile.h>

#include <utils/wizard.h>
#include <utils/macroexpander.h>

namespace ProjectExplorer {

class JsonWizardGenerator;

// Documentation inside.
class PROJECTEXPLORER_EXPORT JsonWizard : public Utils::Wizard
{
    Q_OBJECT

public:
    class GeneratorFile {
    public:
        GeneratorFile() : generator(0) { }
        GeneratorFile(const Core::GeneratedFile &f, JsonWizardGenerator *g) :
            file(f), generator(g)
        { }

        bool isValid() const { return generator; }

        Core::GeneratedFile file;
        JsonWizardGenerator *generator;
    };
    typedef QList<GeneratorFile> GeneratorFiles;
    Q_PROPERTY(GeneratorFiles generateFileList READ generateFileList)

    explicit JsonWizard(QWidget *parent = 0);
    ~JsonWizard();

    void addGenerator(JsonWizardGenerator *gen);

    Utils::MacroExpander *expander();

    GeneratorFiles generateFileList();
    void commitToFileList(const GeneratorFiles &list);

    QString stringValue(const QString &n) const;

    QVariant value(const QString &n) const;
    void setValue(const QString &key, const QVariant &value);

    static bool boolFromVariant(const QVariant &v, Utils::MacroExpander *expander);

    void removeAttributeFromAllFiles(Core::GeneratedFile::Attribute a);

signals:
    void preGenerateFiles(); // emitted before files are generated (can happen several times!)
    void postGenerateFiles(const JsonWizard::GeneratorFiles &files); // emitted after commitToFileList was called.
    void prePromptForOverwrite(const JsonWizard::GeneratorFiles &files); // emitted before overwriting checks are done.
    void preFormatFiles(const JsonWizard::GeneratorFiles &files); // emitted before files are formatted.
    void preWriteFiles(const JsonWizard::GeneratorFiles &files); // emitted before files are written to disk.
    void postProcessFiles(const JsonWizard::GeneratorFiles &files); // emitted before files are post-processed.
    void filesReady(const JsonWizard::GeneratorFiles &files); // emitted just after files are in final state on disk.
    void allDone(const JsonWizard::GeneratorFiles &files); // emitted just after the wizard is done with the files. They are ready to be opened.

public slots:
    void accept();

private slots:
    void handleNewPages(int pageId);
    void handleError(const QString &message);

private:
    QList<JsonWizardGenerator *> m_generators;

    GeneratorFiles m_files;
    Utils::MacroExpander m_expander;
};

} // namespace ProjectExplorer

#endif // JSONWIZARD_H
