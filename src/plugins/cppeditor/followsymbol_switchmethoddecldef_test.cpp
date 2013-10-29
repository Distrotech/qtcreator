/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
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

#include "cppeditor.h"
#include "cppeditorplugin.h"
#include "cppelementevaluator.h"
#include "cppvirtualfunctionassistprovider.h"
#include "cppvirtualfunctionproposalitem.h"

#include <texteditor/codeassist/iassistproposal.h>
#include <texteditor/codeassist/iassistprocessor.h>
#include <texteditor/codeassist/basicproposalitemlistmodel.h>
#include <utils/fileutils.h>

#include <QDebug>
#include <QDir>
#include <QtTest>


/*!
    Tests for Follow Symbol Under Cursor and Switch Between Function Declaration/Definition

    Section numbers refer to

        Working Draft, Standard for Programming Language C++
        Document Number: N3242=11-0012

    You can find potential test code for Follow Symbol Under Cursor on the bottom of this file.
 */
using namespace CPlusPlus;
using namespace CppEditor;
using namespace CppEditor::Internal;
using namespace CppTools;
using namespace TextEditor;
using namespace Core;

class OverrideItem {
public:
    OverrideItem() : line(0) {}
    OverrideItem(const QString &text, int line = 0) : text(text), line(line) {}
    bool isValid() { return line != 0; }

    QString text;
    int line;
};
typedef QList<OverrideItem> OverrideItemList;
Q_DECLARE_METATYPE(OverrideItem)

inline bool operator==(const OverrideItem &lhs, const OverrideItem &rhs)
{
    return lhs.text == rhs.text && lhs.line == rhs.line;
}

QT_BEGIN_NAMESPACE
namespace QTest {
template<> char *toString(const OverrideItem &data)
{
    QByteArray ba = "OverrideItem(";
    ba += data.text.toLatin1() + ", " + QByteArray::number(data.line);
    ba += ")";
    return qstrdup(ba.data());
}
}
QT_END_NAMESPACE

namespace {

/// A fake virtual functions assist provider that runs processor->perform() already in configure()
class VirtualFunctionTestAssistProvider : public VirtualFunctionAssistProvider
{
public:
    VirtualFunctionTestAssistProvider(CPPEditorWidget *editorWidget)
        : m_editorWidget(editorWidget)
    {}

    // Invoke the processor already here to calculate the proposals. Return false in order to
    // indicate that configure failed, so the actual code assist invocation leading to a pop-up
    // will not happen.
    bool configure(const VirtualFunctionAssistProvider::Parameters &params)
    {
        VirtualFunctionAssistProvider::configure(params);

        IAssistProcessor *processor = createProcessor();
        IAssistInterface *assistInterface
                = m_editorWidget->createAssistInterface(FollowSymbol, ExplicitlyInvoked);
        IAssistProposal *immediateProposal = processor->immediateProposal(assistInterface);
        IAssistProposal *finalProposal = processor->perform(assistInterface);

        m_immediateItems = itemList(immediateProposal->model());
        m_finalItems = itemList(finalProposal->model());

        return false;
    }

    static OverrideItemList itemList(IAssistProposalModel *imodel)
    {
        OverrideItemList result;
        BasicProposalItemListModel *model = dynamic_cast<BasicProposalItemListModel *>(imodel);
        if (!model)
            return result;

        // Mimic relevant GenericProposalWidget::showProposal() calls
        model->removeDuplicates();
        model->reset();
        if (model->isSortable(QString()))
            model->sort(QString());

        for (int i = 0, size = model->size(); i < size; ++i) {
            VirtualFunctionProposalItem *item
                = dynamic_cast<VirtualFunctionProposalItem *>(model->proposalItem(i));

            const QString text = model->text(i);
            const int line = item->link().targetLine;
//            Uncomment for updating/generating reference data:
//            qDebug("<< OverrideItem(QLatin1String(\"%s\"), %d)", qPrintable(text), line);
            result << OverrideItem(text, line);
        }

        return result;
    }

public:
    OverrideItemList m_immediateItems;
    OverrideItemList m_finalItems;

private:
    CPPEditorWidget *m_editorWidget;
};

class TestDocument;
typedef QSharedPointer<TestDocument> TestDocumentPtr;

/**
 * Represents a test document.
 *
 * A TestDocument's source can contain special characters:
 *   - a '@' character denotes the initial text cursor position
 *   - a '$' character denotes the target text cursor position
 */
class TestDocument
{
public:
    TestDocument(const QByteArray &theSource, const QString &fileName)
        : source(theSource)
        , fileName(fileName)
        , initialCursorPosition(source.indexOf('@'))
        , targetCursorPosition(source.indexOf('$'))
        , editor(0)
        , editorWidget(0)
    {
        QVERIFY(initialCursorPosition != targetCursorPosition);
        if (initialCursorPosition > targetCursorPosition) {
            source.remove(initialCursorPosition, 1);
            if (targetCursorPosition != -1) {
                source.remove(targetCursorPosition, 1);
                --initialCursorPosition;
            }
        } else {
            source.remove(targetCursorPosition, 1);
            if (initialCursorPosition != -1) {
                source.remove(initialCursorPosition, 1);
                --targetCursorPosition;
            }
        }
    }

    static TestDocumentPtr create(const QByteArray &theOriginalSource, const QString &fileName)
    {
        return TestDocumentPtr(new TestDocument(theOriginalSource, fileName));
    }

    bool hasInitialCursorMarker() const { return initialCursorPosition != -1; }
    bool hasTargetCursorMarker() const { return targetCursorPosition != -1; }

    QString filePath() const
    {
        if (directoryPath.isEmpty())
            qDebug() << "directoryPath not set!";
        return directoryPath + QLatin1Char('/') + fileName;
    }

    void writeToDisk() const
    {
        Utils::FileSaver srcSaver(filePath());
        srcSaver.write(source);
        srcSaver.finalize();
    }

    QByteArray source;

    const QString fileName;
    QString directoryPath;
    int initialCursorPosition;
    int targetCursorPosition;

    CPPEditor *editor;
    CPPEditorWidget *editorWidget;
};

/**
 * Encapsulates the whole process of setting up several editors, positioning the cursor,
 * executing Follow Symbol Under Cursor or Switch Between Function Declaration/Definition
 * and checking the result.
 */
class TestCase
{
public:
    enum CppEditorAction {
        FollowSymbolUnderCursorAction,
        SwitchBetweenMethodDeclarationDefinitionAction
    };

    TestCase(CppEditorAction action, const QByteArray &source,
             const OverrideItemList &expectedVirtualFunctionImmediateProposal = OverrideItemList(),
             const OverrideItemList &expectedVirtualFunctionFinalProposal = OverrideItemList());
    TestCase(CppEditorAction action, const QList<TestDocumentPtr> theTestFiles,
             const OverrideItemList &expectedVirtualFunctionImmediateProposal = OverrideItemList(),
             const OverrideItemList &expectedVirtualFunctionFinalProposal = OverrideItemList());
    ~TestCase();

    void run(bool expectedFail = false);

private:
    TestCase(const TestCase &);
    TestCase &operator=(const TestCase &);

    void init();

    TestDocumentPtr testFileWithInitialCursorMarker();
    TestDocumentPtr testFileWithTargetCursorMarker();

private:
    CppEditorAction m_action;
    QList<TestDocumentPtr> m_testFiles;
    OverrideItemList m_expectedVirtualFunctionImmediateProposal;
    OverrideItemList m_expectedVirtualFunctionFinalProposals;
};

/// Convenience function for creating a TestDocument.
/// See TestDocument.
TestCase::TestCase(CppEditorAction action, const QByteArray &source,
                   const OverrideItemList &expectedVirtualFunctionImmediateProposal,
                   const OverrideItemList &expectedVirtualFunctionFinalProposal)
    : m_action(action)
    , m_expectedVirtualFunctionImmediateProposal(expectedVirtualFunctionImmediateProposal)
    , m_expectedVirtualFunctionFinalProposals(expectedVirtualFunctionFinalProposal)
{
    m_testFiles << TestDocument::create(source, QLatin1String("file.cpp"));
    init();
}

/// Creates a test case with multiple test files.
/// Exactly one test document must be provided that contains '@', the initial position marker.
/// Exactly one test document must be provided that contains '$', the target position marker.
/// It can be the same document.
TestCase::TestCase(CppEditorAction action, const QList<TestDocumentPtr> theTestFiles,
                   const OverrideItemList &expectedVirtualFunctionImmediateProposal,
                   const OverrideItemList &expectedVirtualFunctionFinalProposal)
    : m_action(action)
    , m_testFiles(theTestFiles)
    , m_expectedVirtualFunctionImmediateProposal(expectedVirtualFunctionImmediateProposal)
    , m_expectedVirtualFunctionFinalProposals(expectedVirtualFunctionFinalProposal)
{
    init();
}

void TestCase::init()
{
    // Check if there are initial and target position markers
    QVERIFY2(testFileWithInitialCursorMarker(),
        "No test file with initial cursor marker is provided.");
    QVERIFY2(testFileWithTargetCursorMarker(),
        "No test file with target cursor marker is provided.");

    // Write files to disk
    const QString directoryPath = QDir::tempPath();
    foreach (TestDocumentPtr testFile, m_testFiles) {
        testFile->directoryPath = directoryPath;
        testFile->writeToDisk();
    }

    // Update Code Model
    QStringList filePaths;
    foreach (const TestDocumentPtr &testFile, m_testFiles)
        filePaths << testFile->filePath();
    CppTools::CppModelManagerInterface::instance()->updateSourceFiles(filePaths);

    // Wait for the indexer to process all files.
    // All these files are "Fast Checked", that is the function bodies are not processed.
    QStringList filePathsNotYetInSnapshot(filePaths);
    forever {
        Snapshot snapshot = CppTools::CppModelManagerInterface::instance()->snapshot();
        foreach (const QString &filePath, filePathsNotYetInSnapshot) {
            if (snapshot.contains(filePath))
                filePathsNotYetInSnapshot.removeOne(filePath);
        }
        if (filePathsNotYetInSnapshot.isEmpty())
            break;
        QCoreApplication::processEvents();
    }

    // Open Files
    foreach (TestDocumentPtr testFile, m_testFiles) {
        testFile->editor
            = dynamic_cast<CPPEditor *>(EditorManager::openEditor(testFile->filePath()));
        QVERIFY(testFile->editor);

        testFile->editorWidget = dynamic_cast<CPPEditorWidget *>(testFile->editor->editorWidget());
        QVERIFY(testFile->editorWidget);

        // Wait until the indexer processed the just opened file.
        // The file is "Full Checked" since it is in the working copy now,
        // that is the function bodies are processed.
        forever {
            Snapshot snapshot = CppTools::CppModelManagerInterface::instance()->snapshot();
            if (Document::Ptr document = snapshot.document(testFile->filePath())) {
                if (document->checkMode() == Document::FullCheck)
                    break;
                QCoreApplication::processEvents();
            }
        }

        // Rehighlight
        testFile->editorWidget->semanticRehighlight(true);
        // Wait for the semantic info from the future
        while (testFile->editorWidget->semanticInfo().doc.isNull())
            QCoreApplication::processEvents();
    }
}

TestCase::~TestCase()
{
    // Close editors
    QList<Core::IEditor *> editorsToClose;
    foreach (const TestDocumentPtr testFile, m_testFiles) {
        if (testFile->editor)
            editorsToClose << testFile->editor;
    }
    EditorManager::closeEditors(editorsToClose, false);
    QCoreApplication::processEvents(); // process any pending events

    // Remove the test files from the code-model
    CppModelManagerInterface *mmi = CppTools::CppModelManagerInterface::instance();
    mmi->GC();
    QCOMPARE(mmi->snapshot().size(), 0);
}

TestDocumentPtr TestCase::testFileWithInitialCursorMarker()
{
    foreach (const TestDocumentPtr testFile, m_testFiles) {
        if (testFile->hasInitialCursorMarker())
            return testFile;
    }
    return TestDocumentPtr();
}

TestDocumentPtr TestCase::testFileWithTargetCursorMarker()
{
    foreach (const TestDocumentPtr testFile, m_testFiles) {
        if (testFile->hasTargetCursorMarker())
            return testFile;
    }
    return TestDocumentPtr();
}

void TestCase::run(bool expectedFail)
{
    TestDocumentPtr initialTestFile = testFileWithInitialCursorMarker();
    QVERIFY(initialTestFile);
    TestDocumentPtr targetTestFile = testFileWithTargetCursorMarker();
    QVERIFY(targetTestFile);

    // Activate editor of initial test file
    EditorManager::activateEditor(initialTestFile->editor);

    initialTestFile->editor->setCursorPosition(initialTestFile->initialCursorPosition);
//    qDebug() << "Initial line:" << initialTestFile->editor->currentLine();
//    qDebug() << "Initial column:" << initialTestFile->editor->currentColumn() - 1;

    OverrideItemList immediateVirtualSymbolResults;
    OverrideItemList finalVirtualSymbolResults;

    // Trigger the action
    switch (m_action) {
    case FollowSymbolUnderCursorAction: {
        CPPEditorWidget *widget = initialTestFile->editorWidget;
        FollowSymbolUnderCursor *delegate = widget->followSymbolUnderCursorDelegate();
        VirtualFunctionAssistProvider *original = delegate->virtualFunctionAssistProvider();

        // Set test provider, run and get results
        QScopedPointer<VirtualFunctionTestAssistProvider> testProvider(
            new VirtualFunctionTestAssistProvider(widget));
        delegate->setVirtualFunctionAssistProvider(testProvider.data());
        initialTestFile->editorWidget->openLinkUnderCursor();
        immediateVirtualSymbolResults = testProvider->m_immediateItems;
        finalVirtualSymbolResults = testProvider->m_finalItems;

        // Restore original test provider
        delegate->setVirtualFunctionAssistProvider(original);
        break;
    }
    case SwitchBetweenMethodDeclarationDefinitionAction:
        CppEditorPlugin::instance()->switchDeclarationDefinition();
        break;
    default:
        QFAIL("Unknown test action");
        break;
    }

    QCoreApplication::processEvents();

    // Compare
    IEditor *currentEditor = EditorManager::currentEditor();
    BaseTextEditor *currentTextEditor = dynamic_cast<BaseTextEditor*>(currentEditor);
    QVERIFY(currentTextEditor);

    QCOMPARE(currentTextEditor->document()->filePath(), targetTestFile->filePath());
    int expectedLine, expectedColumn;
    currentTextEditor->convertPosition(targetTestFile->targetCursorPosition,
                                       &expectedLine, &expectedColumn);
//    qDebug() << "Expected line:" << expectedLine;
//    qDebug() << "Expected column:" << expectedColumn;

    if (expectedFail)
        QEXPECT_FAIL("", "Contributor works on a fix.", Abort);
    QCOMPARE(currentTextEditor->currentLine(), expectedLine);
    QCOMPARE(currentTextEditor->currentColumn() - 1, expectedColumn);

//    qDebug() << immediateVirtualSymbolResults;
//    qDebug() << finalVirtualSymbolResults;
    QCOMPARE(immediateVirtualSymbolResults, m_expectedVirtualFunctionImmediateProposal);
    QCOMPARE(finalVirtualSymbolResults, m_expectedVirtualFunctionFinalProposals);
}

} // anonymous namespace

void CppEditorPlugin::test_SwitchMethodDeclarationDefinition_fromFunctionDeclarationSymbol()
{
    QList<TestDocumentPtr> testFiles;

    const QByteArray headerContents =
        "class C\n"
        "{\n"
        "public:\n"
        "    C();\n"
        "    int @function();\n"  // Line 5
        "};\n"
        ;
    testFiles << TestDocument::create(headerContents, QLatin1String("file.h"));

    const QByteArray sourceContents =
        "#include \"file.h\"\n"
        "\n"
        "C::C()\n"
        "{\n"
        "}\n"                   // Line 5
        "\n"
        "int C::$function()\n"
        "{\n"
        "    return 1 + 1;\n"
        "}\n"                   // Line 10
        ;
    testFiles << TestDocument::create(sourceContents, QLatin1String("file.cpp"));

    TestCase test(TestCase::SwitchBetweenMethodDeclarationDefinitionAction, testFiles);
    test.run();
}

void CppEditorPlugin::test_SwitchMethodDeclarationDefinition_fromFunctionDefinitionSymbol()
{
    QList<TestDocumentPtr> testFiles;

    const QByteArray headerContents =
        "class C\n"
        "{\n"
        "public:\n"
        "    C();\n"
        "    int $function();\n"
        "};\n"
        ;
    testFiles << TestDocument::create(headerContents, QLatin1String("file.h"));

    const QByteArray sourceContents =
        "#include \"file.h\"\n"
        "\n"
        "C::C()\n"
        "{\n"
        "}\n"
        "\n"
        "int C::@function()\n"
        "{\n"
        "    return 1 + 1;\n"
        "}\n"
        ;
    testFiles << TestDocument::create(sourceContents, QLatin1String("file.cpp"));

    TestCase test(TestCase::SwitchBetweenMethodDeclarationDefinitionAction, testFiles);
    test.run();
}

void CppEditorPlugin::test_SwitchMethodDeclarationDefinition_fromFunctionBody()
{
    QList<TestDocumentPtr> testFiles;

    const QByteArray headerContents =
        "class C\n"
        "{\n"
        "public:\n"
        "    C();\n"
        "    int $function();\n"
        "};\n"
        ;
    testFiles << TestDocument::create(headerContents, QLatin1String("file.h"));

    const QByteArray sourceContents =
        "#include \"file.h\"\n"
        "\n"
        "C::C()\n"
        "{\n"
        "}\n"                   // Line 5
        "\n"
        "int C::function()\n"
        "{\n"
        "    return @1 + 1;\n"
        "}\n"                   // Line 10
        ;
    testFiles << TestDocument::create(sourceContents, QLatin1String("file.cpp"));

    TestCase test(TestCase::SwitchBetweenMethodDeclarationDefinitionAction, testFiles);
    test.run();
}

void CppEditorPlugin::test_SwitchMethodDeclarationDefinition_fromReturnType()
{
    QList<TestDocumentPtr> testFiles;

    const QByteArray headerContents =
        "class C\n"
        "{\n"
        "public:\n"
        "    C();\n"
        "    int $function();\n"
        "};\n"
        ;
    testFiles << TestDocument::create(headerContents, QLatin1String("file.h"));

    const QByteArray sourceContents =
        "#include \"file.h\"\n"
        "\n"
        "C::C()\n"
        "{\n"
        "}\n"                   // Line 5
        "\n"
        "@int C::function()\n"
        "{\n"
        "    return 1 + 1;\n"
        "}\n"                   // Line 10
        ;
    testFiles << TestDocument::create(sourceContents, QLatin1String("file.cpp"));

    TestCase test(TestCase::SwitchBetweenMethodDeclarationDefinitionAction, testFiles);
    test.run();
}

/// Check ...
void CppEditorPlugin::test_FollowSymbolUnderCursor_globalVarFromFunction()
{
    const QByteArray source =
        "int $j;\n"
        "int main()\n"
        "{\n"
        "    @j = 2;\n"
        "}\n"           // Line 5
        ;

    TestCase test(TestCase::FollowSymbolUnderCursorAction, source);
    test.run();
}

void CppEditorPlugin::test_FollowSymbolUnderCursor_funLocalVarHidesClassMember()
{
    // 3.3.10 Name hiding (par 3.), from text
    const QByteArray source =
        "struct C {\n"
        "    void f()\n"
        "    {\n"
        "        int $member; // hides C::member\n"
        "        ++@member;\n"                       // Line 5
        "    }\n"
        "    int member;\n"
        "};\n"
        ;

    TestCase test(TestCase::FollowSymbolUnderCursorAction, source);
    test.run();
}

void CppEditorPlugin::test_FollowSymbolUnderCursor_funLocalVarHidesNamespaceMemberIntroducedByUsingDirective()
{
    // 3.3.10 Name hiding (par 4.), from text
    const QByteArray source =
        "namespace N {\n"
        "    int i;\n"
        "}\n"
        "\n"
        "int main()\n"                       // Line 5
        "{\n"
        "    using namespace N;\n"
        "    int $i;\n"
        "    ++i@; // refers to local i;\n"
        "}\n"                                // Line 10
        ;

    TestCase test(TestCase::FollowSymbolUnderCursorAction, source);
    test.run();
}

void CppEditorPlugin::test_FollowSymbolUnderCursor_loopLocalVarHidesOuterScopeVariable1()
{
    // 3.3.3 Block scope (par. 4), from text
    // Same for if, while, switch
    const QByteArray source =
        "int main()\n"
        "{\n"
        "    int i = 1;\n"
        "    for (int $i = 0; i < 10; ++i) { // 'i' refers to for's i\n"
        "        i = @i; // same\n"                                       // Line 5
        "    }\n"
        "}\n";
        ;

    TestCase test(TestCase::FollowSymbolUnderCursorAction, source);
    test.run();
}

void CppEditorPlugin::test_FollowSymbolUnderCursor_loopLocalVarHidesOuterScopeVariable2()
{
    // 3.3.3 Block scope (par. 4), from text
    // Same for if, while, switch
    const QByteArray source =
        "int main()\n"
        "{\n"
        "    int i = 1;\n"
        "    for (int $i = 0; @i < 10; ++i) { // 'i' refers to for's i\n"
        "        i = i; // same\n"                                         // Line 5
        "    }\n"
        "}\n";
        ;

    TestCase test(TestCase::FollowSymbolUnderCursorAction, source);
    test.run();
}

void CppEditorPlugin::test_FollowSymbolUnderCursor_subsequentDefinedClassMember()
{
    // 3.3.7 Class scope, part of the example
    const QByteArray source =
        "class X {\n"
        "    int f() { return @i; } // i refers to class's i\n"
        "    int $i;\n"
        "};\n"
        ;

    TestCase test(TestCase::FollowSymbolUnderCursorAction, source);
    test.run();
}

void CppEditorPlugin::test_FollowSymbolUnderCursor_classMemberHidesOuterTypeDef()
{
    // 3.3.7 Class scope, part of the example
    // Variable name hides type name.
    const QByteArray source =
        "typedef int c;\n"
        "class X {\n"
        "    int f() { return @c; } // c refers to class' c\n"
        "    int $c; // hides typedef name\n"
        "};\n"                                                 // Line 5
        ;

    TestCase test(TestCase::FollowSymbolUnderCursorAction, source);
    test.run();
}

void CppEditorPlugin::test_FollowSymbolUnderCursor_globalVarFromEnum()
{
    // 3.3.2 Point of declaration (par. 1), copy-paste
    const QByteArray source =
        "const int $x = 12;\n"
        "int main()\n"
        "{\n"
        "    enum { x = @x }; // x refers to global x\n"
        "}\n"                                             // Line 5
        ;

    TestCase test(TestCase::FollowSymbolUnderCursorAction, source);
    test.run(/*expectedFail =*/ true);
}

void CppEditorPlugin::test_FollowSymbolUnderCursor_selfInitialization()
{
    // 3.3.2 Point of declaration
    const QByteArray source =
        "int x = 12;\n"
        "int main()\n"
        "{\n"
        "   int $x = @x; // Second x refers to local x\n"
        "}\n"                                              // Line 5
        ;

    TestCase test(TestCase::FollowSymbolUnderCursorAction, source);
    test.run();
}

void CppEditorPlugin::test_FollowSymbolUnderCursor_pointerToClassInClassDefinition()
{
    // 3.3.2 Point of declaration (par. 3), from text
    const QByteArray source =
        "class $Foo {\n"
        "    @Foo *p; // Refers to above Foo\n"
        "};\n"
        ;

    TestCase test(TestCase::FollowSymbolUnderCursorAction, source);
    test.run();
}

void CppEditorPlugin::test_FollowSymbolUnderCursor_previouslyDefinedMemberFromArrayDefinition()
{
    // 3.3.2 Point of declaration (par. 5), copy-paste
    const QByteArray source =
        "struct X {\n"
        "    enum E { $z = 16 };\n"
        "    int b[X::@z]; // z refers to defined z\n"
        "};\n"
        ;

    TestCase test(TestCase::FollowSymbolUnderCursorAction, source);
    test.run();
}

void CppEditorPlugin::test_FollowSymbolUnderCursor_outerStaticMemberVariableFromInsideSubclass()
{
    // 3.3.7 Class scope (par. 2), from text
    const QByteArray source =
        "struct C\n"
        "{\n"
        "   struct I\n"
        "   {\n"
        "       void f()\n"                            // Line 5
        "       {\n"
        "           int i = @c; // refers to C's c\n"
        "       }\n"
        "   };\n"
        "\n"                                           // Line 10
        "   static int $c;\n"
        "};\n"
        ;

    TestCase test(TestCase::FollowSymbolUnderCursorAction, source);
    test.run();
}

void CppEditorPlugin::test_FollowSymbolUnderCursor_memberVariableFollowingDotOperator()
{
    // 3.3.7 Class scope (par. 1), part of point 5
    const QByteArray source =
        "struct C\n"
        "{\n"
        "    int $member;\n"
        "};\n"
        "\n"                 // Line 5
        "int main()\n"
        "{\n"
        "    C c;\n"
        "    c.@member++;\n"
        "}\n"                // Line 10
        ;

    TestCase test(TestCase::FollowSymbolUnderCursorAction, source);
    test.run();
}

void CppEditorPlugin::test_FollowSymbolUnderCursor_memberVariableFollowingArrowOperator()
{
    // 3.3.7 Class scope (par. 1), part of point 5
    const QByteArray source =
        "struct C\n"
        "{\n"
        "    int $member;\n"
        "};\n"
        "\n"                    // Line 5
        "int main()\n"
        "{\n"
        "    C* c;\n"
        "    c->@member++;\n"
        "}\n"                   // Line 10
        ;

    TestCase test(TestCase::FollowSymbolUnderCursorAction, source);
    test.run();
}

void CppEditorPlugin::test_FollowSymbolUnderCursor_staticMemberVariableFollowingScopeOperator()
{
    // 3.3.7 Class scope (par. 1), part of point 5
    const QByteArray source =
        "struct C\n"
        "{\n"
        "    static int $member;\n"
        "};\n"
        "\n"                        // Line 5
        "int main()\n"
        "{\n"
        "    C::@member++;\n"
        "}\n"
        ;

    TestCase test(TestCase::FollowSymbolUnderCursorAction, source);
    test.run();
}

void CppEditorPlugin::test_FollowSymbolUnderCursor_staticMemberVariableFollowingDotOperator()
{
    // 3.3.7 Class scope (par. 2), from text
    const QByteArray source =
        "struct C\n"
        "{\n"
        "    static int $member;\n"
        "};\n"
        "\n"                        // Line 5
        "int main()\n"
        "{\n"
        "    C c;\n"
        "    c.@member;\n"
        "}\n"                       // Line 10
        ;

    TestCase test(TestCase::FollowSymbolUnderCursorAction, source);
    test.run();
}


void CppEditorPlugin::test_FollowSymbolUnderCursor_staticMemberVariableFollowingArrowOperator()
{
    // 3.3.7 Class scope (par. 2), from text
    const QByteArray source =
        "struct C\n"
        "{\n"
        "    static int $member;\n"
        "};\n"
        "\n"                         // Line 5
        "int main()\n"
        "{\n"
        "    C *c;\n"
        "    c->@member++;\n"
        "}\n"                        // Line 10
        ;

    TestCase test(TestCase::FollowSymbolUnderCursorAction, source);
    test.run();
}

void CppEditorPlugin::test_FollowSymbolUnderCursor_previouslyDefinedEnumValueFromInsideEnum()
{
    // 3.3.8 Enumeration scope
    const QByteArray source =
        "enum {\n"
        "    $i = 0,\n"
        "    j = @i // refers to i above\n"
        "};\n"
        ;

    TestCase test(TestCase::FollowSymbolUnderCursorAction, source);
    test.run();
}

void CppEditorPlugin::test_FollowSymbolUnderCursor_nsMemberHidesNsMemberIntroducedByUsingDirective()
{
    // 3.3.8 Enumeration scope
    const QByteArray source =
        "namespace A {\n"
        "  char x;\n"
        "}\n"
        "\n"
        "namespace B {\n"                               // Line 5
        "  using namespace A;\n"
        "  int $x; // hides A::x\n"
        "}\n"
        "\n"
        "int main()\n"                                  // Line 10
        "{\n"
        "    B::@x++; // refers to B's X, not A::x\n"
        "}\n"
        ;

    TestCase test(TestCase::FollowSymbolUnderCursorAction, source);
    test.run();
}

void CppEditorPlugin::test_FollowSymbolUnderCursor_baseClassFunctionIntroducedByUsingDeclaration()
{
    // 3.3.10 Name hiding, from text
    // www.stroustrup.com/bs_faq2.html#overloadderived
    const QByteArray source =
        "struct B {\n"
        "    int $f(int) {}\n"
        "};\n"
        "\n"
        "class D : public B {\n"                              // Line 5
        "public:\n"
        "    using B::f; // make every f from B available\n"
        "    double f(double) {}\n"
        "};\n"
        "\n"                                                  // Line 10
        "int main()\n"
        "{\n"
        "    D* pd = new D;\n"
        "    pd->@f(2); // refers to B::f\n"
        "    pd->f(2.3); // refers to D::f\n"                 // Line 15
        "}\n"
        ;

    TestCase test(TestCase::FollowSymbolUnderCursorAction, source);
    test.run();
}

void CppEditorPlugin::test_FollowSymbolUnderCursor_funWithSameNameAsBaseClassFunIntroducedByUsingDeclaration()
{
    // 3.3.10 Name hiding, from text
    // www.stroustrup.com/bs_faq2.html#overloadderived
    const QByteArray source =
        "struct B {\n"
        "    int f(int) {}\n"
        "};\n"
        "\n"
        "class D : public B {\n"                              // Line 5
        "public:\n"
        "    using B::f; // make every f from B available\n"
        "    double $f(double) {}\n"
        "};\n"
        "\n"                                                  // Line 10
        "int main()\n"
        "{\n"
        "    D* pd = new D;\n"
        "    pd->f(2); // refers to B::f\n"
        "    pd->@f(2.3); // refers to D::f\n"                // Line 15
        "}\n"
        ;

    TestCase test(TestCase::FollowSymbolUnderCursorAction, source);
    test.run();
}

void CppEditorPlugin::test_FollowSymbolUnderCursor_funLocalVarHidesOuterClass()
{
    // 3.3.10 Name hiding (par 2.), from text
    // A class name (9.1) or enumeration name (7.2) can be hidden by the name of a variable, data member,
    // function, or enumerator declared in the same scope.
    const QByteArray source =
        "struct C {};\n"
        "\n"
        "int main()\n"
        "{\n"
        "    int $C; // hides type C\n"  // Line 5
        "    ++@C;\n"
        "}\n"
        ;

    TestCase test(TestCase::FollowSymbolUnderCursorAction, source);
    test.run();
}

void CppEditorPlugin::test_FollowSymbolUnderCursor_classConstructor()
{
    const QByteArray source =
            "class Foo {\n"
            "    F@oo();"
            "    ~Foo();"
            "};\n\n"
            "Foo::$Foo()\n"
            "{\n"
            "}\n\n"
            "Foo::~Foo()\n"
            "{\n"
            "}\n";

    TestCase test(TestCase::FollowSymbolUnderCursorAction, source);
    test.run();
}

void CppEditorPlugin::test_FollowSymbolUnderCursor_classDestructor()
{
    const QByteArray source =
            "class Foo {\n"
            "    Foo();"
            "    ~@Foo();"
            "};\n\n"
            "Foo::Foo()\n"
            "{\n"
            "}\n\n"
            "Foo::~$Foo()\n"
            "{\n"
            "}\n";

    TestCase test(TestCase::FollowSymbolUnderCursorAction, source);
    test.run();
}

void CppEditorPlugin::test_FollowSymbolUnderCursor_QObject_connect_data()
{
#define TAG(str) secondQObjectParam ? str : str ", no 2nd QObject"
    QTest::addColumn<char>("start");
    QTest::addColumn<char>("target");
    QTest::addColumn<bool>("secondQObjectParam");
    for (int i = 0; i < 2; ++i) {
        bool secondQObjectParam = (i == 0);
        QTest::newRow(TAG("SIGNAL: before keyword"))
                << '1' << '1' << secondQObjectParam;
        QTest::newRow(TAG("SIGNAL: in keyword"))
                << '2' << '1' << secondQObjectParam;
        QTest::newRow(TAG("SIGNAL: before parenthesis"))
                << '3' << '1' << secondQObjectParam;
        QTest::newRow(TAG("SIGNAL: before identifier"))
                << '4' << '1' << secondQObjectParam;
        QTest::newRow(TAG("SIGNAL: in identifier"))
                << '5' << '1' << secondQObjectParam;
        QTest::newRow(TAG("SIGNAL: before identifier parenthesis"))
                << '6' << '1' << secondQObjectParam;
        QTest::newRow(TAG("SLOT: before keyword"))
                << '7' << '2' << secondQObjectParam;
        QTest::newRow(TAG("SLOT: in keyword"))
                << '8' << '2' << secondQObjectParam;
        QTest::newRow(TAG("SLOT: before parenthesis"))
                << '9' << '2' << secondQObjectParam;
        QTest::newRow(TAG("SLOT: before identifier"))
                << 'A' << '2' << secondQObjectParam;
        QTest::newRow(TAG("SLOT: in identifier"))
                << 'B' << '2' << secondQObjectParam;
        QTest::newRow(TAG("SLOT: before identifier parenthesis"))
                << 'C' << '2' << secondQObjectParam;
    }
#undef TAG
}

static void selectMarker(QByteArray *source, char marker, char number)
{
    int idx = 0;
    forever {
        idx = source->indexOf(marker, idx);
        if (idx == -1)
            break;
        if (source->at(idx + 1) == number) {
            ++idx;
            source->remove(idx, 1);
        } else {
            source->remove(idx, 2);
        }
    }
}

void CppEditorPlugin::test_FollowSymbolUnderCursor_QObject_connect()
{
    QFETCH(char, start);
    QFETCH(char, target);
    QFETCH(bool, secondQObjectParam);
    QByteArray source =
            "class Foo : public QObject\n"
            "{\n"
            "signals:\n"
            "    void $1endOfWorld();\n"
            "public slots:\n"
            "    void $2onWorldEnded()\n"
            "    {\n"
            "    }\n"
            "};\n"
            "\n"
            "void bla()\n"
            "{\n"
            "   Foo foo;\n"
            "   connect(&foo, @1SI@2GNAL@3(@4end@5OfWorld@6()),\n"
            "           &foo, @7SL@8OT@9(@Aon@BWorldEnded@C()));\n"
            "}\n";

    selectMarker(&source, '@', start);
    selectMarker(&source, '$', target);

    if (!secondQObjectParam)
        source.replace(" &foo, ", QByteArray());

    if (start >= '7' && !secondQObjectParam) {
        qWarning("SLOT jump triggers QTCREATORBUG-10265. Skipping.");
        return;
    }

    TestCase test(TestCase::FollowSymbolUnderCursorAction, source);
    test.run();
}

void CppEditorPlugin::test_FollowSymbolUnderCursor_classOperator_onOperatorToken_data()
{
    QTest::addColumn<bool>("toDeclaration");
    QTest::newRow("forward") << false;
    QTest::newRow("backward") << true;
}

void CppEditorPlugin::test_FollowSymbolUnderCursor_classOperator_onOperatorToken()
{
    QFETCH(bool, toDeclaration);

    QByteArray source =
            "class Foo {\n"
            "    void @operator[](size_t idx);\n"
            "};\n\n"
            "void Foo::$operator[](size_t idx)\n"
            "{\n"
            "}\n";
    if (toDeclaration)
        source.replace('@', '#').replace('$', '@').replace('#', '$');
    TestCase test(TestCase::FollowSymbolUnderCursorAction, source);
    test.run();
}

void CppEditorPlugin::test_FollowSymbolUnderCursor_classOperator_data()
{
    test_FollowSymbolUnderCursor_classOperator_onOperatorToken_data();
}

void CppEditorPlugin::test_FollowSymbolUnderCursor_classOperator()
{
    QFETCH(bool, toDeclaration);

    QByteArray source =
            "class Foo {\n"
            "    void $2operator@1[](size_t idx);\n"
            "};\n\n"
            "void Foo::$1operator@2[](size_t idx)\n"
            "{\n"
            "}\n";
    if (toDeclaration)
        source.replace("@1", QByteArray()).replace("$1", QByteArray())
                .replace("@2", "@").replace("$2", "$");
    else
        source.replace("@2", QByteArray()).replace("$2", QByteArray())
                .replace("@1", "@").replace("$1", "$");
    TestCase test(TestCase::FollowSymbolUnderCursorAction, source);
    test.run();
}

void CppEditorPlugin::test_FollowSymbolUnderCursor_classOperator_inOp_data()
{
    test_FollowSymbolUnderCursor_classOperator_onOperatorToken_data();
}

void CppEditorPlugin::test_FollowSymbolUnderCursor_classOperator_inOp()
{
    QFETCH(bool, toDeclaration);

    QByteArray source =
            "class Foo {\n"
            "    void $2operator[@1](size_t idx);\n"
            "};\n\n"
            "void Foo::$1operator[@2](size_t idx)\n"
            "{\n"
            "}\n";
    if (toDeclaration)
        source.replace("@1", QByteArray()).replace("$1", QByteArray())
                .replace("@2", "@").replace("$2", "$");
    else
        source.replace("@2", QByteArray()).replace("$2", QByteArray())
                .replace("@1", "@").replace("$1", "$");
    TestCase test(TestCase::FollowSymbolUnderCursorAction, source);
    test.run();
}

void CppEditorPlugin::test_FollowSymbolUnderCursor_using_QTCREATORBUG7903_globalNamespace()
{
    const QByteArray source =
            "namespace NS {\n"
            "class Foo {};\n"
            "}\n"
            "using NS::$Foo;\n"
            "void fun()\n"
            "{\n"
            "    @Foo foo;\n"
            "}\n"
        ;

    TestCase test(TestCase::FollowSymbolUnderCursorAction, source);
    test.run();
}

void CppEditorPlugin::test_FollowSymbolUnderCursor_using_QTCREATORBUG7903_namespace()
{
    const QByteArray source =
            "namespace NS {\n"
            "class Foo {};\n"
            "}\n"
            "namespace NS1 {\n"
            "void fun()\n"
            "{\n"
            "    using NS::$Foo;\n"
            "    @Foo foo;\n"
            "}\n"
            "}\n"
        ;

    TestCase test(TestCase::FollowSymbolUnderCursorAction, source);
    test.run();
}

void CppEditorPlugin::test_FollowSymbolUnderCursor_using_QTCREATORBUG7903_insideFunction()
{
    const QByteArray source =
            "namespace NS {\n"
            "class Foo {};\n"
            "}\n"
            "void fun()\n"
            "{\n"
            "    using NS::$Foo;\n"
            "    @Foo foo;\n"
            "}\n"
        ;

    TestCase test(TestCase::FollowSymbolUnderCursorAction, source);
    test.run();
}

/// Check: Static type is base class pointer, all overrides are presented.
void CppEditorPlugin::test_FollowSymbolUnderCursor_virtualFunctionCall_allOverrides()
{
    const QByteArray source =
            "struct A { virtual void virt() = 0; };\n"
            "void A::virt() {}\n"
            "\n"
            "struct B : A { void virt(); };\n"
            "void B::virt() {}\n"
            "\n"
            "struct C : B { void virt(); };\n"
            "void C::virt() {}\n"
            "\n"
            "struct CD1 : C { void virt(); };\n"
            "void CD1::virt() {}\n"
            "\n"
            "struct CD2 : C { void virt(); };\n"
            "void CD2::virt() {}\n"
            "\n"
            "int f(A *o) { o->$@virt(); }\n"
            "}\n"
            ;

    const OverrideItemList immediateResults = OverrideItemList()
            << OverrideItem(QLatin1String("A::virt"), 2)
            << OverrideItem(QLatin1String("...searching overrides"));
    const OverrideItemList finalResults = OverrideItemList()
            << OverrideItem(QLatin1String("A::virt"), 2)
            << OverrideItem(QLatin1String("B::virt"), 5)
            << OverrideItem(QLatin1String("C::virt"), 8)
            << OverrideItem(QLatin1String("CD1::virt"), 11)
            << OverrideItem(QLatin1String("CD2::virt"), 14);

    TestCase test(TestCase::FollowSymbolUnderCursorAction, source, immediateResults, finalResults);
    test.run();
}

/// Check: Static type is derived class pointer, only overrides of sub classes are presented.
void CppEditorPlugin::test_FollowSymbolUnderCursor_virtualFunctionCall_possibleOverrides1()
{
    const QByteArray source =
            "struct A { virtual void virt() = 0; };\n"
            "void A::virt() {}\n"
            "\n"
            "struct B : A { void virt(); };\n"
            "void B::virt() {}\n"
            "\n"
            "struct C : B { void virt(); };\n"
            "void C::virt() {}\n"
            "\n"
            "struct CD1 : C { void virt(); };\n"
            "void CD1::virt() {}\n"
            "\n"
            "struct CD2 : C { void virt(); };\n"
            "void CD2::virt() {}\n"
            "\n"
            "int f(B *o) { o->$@virt(); }\n"
            "}\n"
            ;

    const OverrideItemList immediateResults = OverrideItemList()
            << OverrideItem(QLatin1String("B::virt"), 5)
            << OverrideItem(QLatin1String("...searching overrides"));
    const OverrideItemList finalResults = OverrideItemList()
            << OverrideItem(QLatin1String("B::virt"), 5)
            << OverrideItem(QLatin1String("C::virt"), 8)
            << OverrideItem(QLatin1String("CD1::virt"), 11)
            << OverrideItem(QLatin1String("CD2::virt"), 14);

    TestCase test(TestCase::FollowSymbolUnderCursorAction, source, immediateResults, finalResults);
    test.run();
}

/// Check: Virtual function call in member of class hierarchy, only possible overrides are presented.
void CppEditorPlugin::test_FollowSymbolUnderCursor_virtualFunctionCall_possibleOverrides2()
{
    const QByteArray source =
            "struct A { virtual void virt(); };\n"
            "void A::virt() {}\n"
            "\n"
            "struct B : public A { void virt(); };\n"
            "void B::virt() {}\n"
            "\n"
            "struct C : public B { void g() { virt$@(); } }; \n"
            "\n"
            "struct D : public C { void virt(); };\n"
            "void D::virt() {}\n"
            ;

    const OverrideItemList immediateResults = OverrideItemList()
            << OverrideItem(QLatin1String("B::virt"), 5)
            << OverrideItem(QLatin1String("...searching overrides"));
    const OverrideItemList finalResults = OverrideItemList()
            << OverrideItem(QLatin1String("B::virt"), 5)
            << OverrideItem(QLatin1String("D::virt"), 10);

    TestCase test(TestCase::FollowSymbolUnderCursorAction, source, immediateResults, finalResults);
    test.run();
}

/// Check: If no definition is found, fallback to the declaration.
void CppEditorPlugin::test_FollowSymbolUnderCursor_virtualFunctionCall_fallbackToDeclaration()
{
    const QByteArray source =
            "struct A { virtual void virt(); };\n"
            "\n"
            "int f(A *o) { o->$@virt(); }\n"
            ;

    const OverrideItemList immediateResults = OverrideItemList()
            << OverrideItem(QLatin1String("A::virt"), 1)
            << OverrideItem(QLatin1String("...searching overrides"));
    const OverrideItemList finalResults = OverrideItemList()
            << OverrideItem(QLatin1String("A::virt"), 1);

    TestCase test(TestCase::FollowSymbolUnderCursorAction, source, immediateResults, finalResults);
    test.run();
}

/// Check: Ensure that the first entry in the final results is the same as the first in the
///        immediate results.
void CppEditorPlugin::test_FollowSymbolUnderCursor_virtualFunctionCall_itemOrder()
{
    const QByteArray source =
            "struct C { virtual void virt() = 0; };\n"
            "void C::virt() {}\n"
            "\n"
            "struct B : C { void virt(); };\n"
            "void B::virt() {}\n"
            "\n"
            "struct A : B { void virt(); };\n"
            "void A::virt() {}\n"
            "\n"
            "int f(C *o) { o->$@virt(); }\n"
            ;

    const OverrideItemList immediateResults = OverrideItemList()
            << OverrideItem(QLatin1String("C::virt"), 2)
            << OverrideItem(QLatin1String("...searching overrides"));
    const OverrideItemList finalResults = OverrideItemList()
            << OverrideItem(QLatin1String("C::virt"), 2)
            << OverrideItem(QLatin1String("A::virt"), 8)
            << OverrideItem(QLatin1String("B::virt"), 5);

    TestCase test(TestCase::FollowSymbolUnderCursorAction, source, immediateResults, finalResults);
    test.run();
}

/// Check: Trigger on a.virt() if a is of type &A.
void CppEditorPlugin::test_FollowSymbolUnderCursor_virtualFunctionCall_onDotMemberAccessOfReferenceTypes()
{
    const QByteArray source =
            "struct A { virtual void virt() = 0; };\n"
            "void A::virt() {}\n"
            "\n"
            "void client(A &o) { o.$@virt(); }\n"
            ;

    const OverrideItemList immediateResults = OverrideItemList()
            << OverrideItem(QLatin1String("A::virt"), 2)
            << OverrideItem(QLatin1String("...searching overrides"));
    const OverrideItemList finalResults = OverrideItemList()
            << OverrideItem(QLatin1String("A::virt"), 2);

    TestCase test(TestCase::FollowSymbolUnderCursorAction, source, immediateResults, finalResults);
    test.run();
}

/// Check: Do not trigger on a.virt() if a is of type A.
void CppEditorPlugin::test_FollowSymbolUnderCursor_virtualFunctionCall_notOnDotMemberAccessOfNonReferenceType()
{
    const QByteArray source =
            "struct A { virtual void virt(); };\n"
            "void A::$virt() {}\n"
            "\n"
            "void client(A o) { o.@virt(); }\n"
            ;

    TestCase test(TestCase::FollowSymbolUnderCursorAction, source);
    test.run();
}

/// Check: Do not trigger on qualified function calls.
void CppEditorPlugin::test_FollowSymbolUnderCursor_virtualFunctionCall_notOnQualified()
{
    const QByteArray source =
            "struct A { virtual void virt(); };\n"
            "void A::$virt() {}\n"
            "\n"
            "struct B : public A {\n"
            "    void virt();\n"
            "    void g() { A::@virt(); }\n"
            "};\n"
            ;

    TestCase test(TestCase::FollowSymbolUnderCursorAction, source);
    test.run();
}

/// Check: Do not trigger on member function declaration.
void CppEditorPlugin::test_FollowSymbolUnderCursor_virtualFunctionCall_notOnDeclaration()
{
    const QByteArray source =
            "struct A { virtual void virt(); };\n"
            "void A::virt() {}\n"
            "\n"
            "struct B : public A { void virt@(); };\n"
            "void B::$virt() {}\n"
            ;

    TestCase test(TestCase::FollowSymbolUnderCursorAction, source);
    test.run();
}

/// Check: Do not trigger on function definition.
void CppEditorPlugin::test_FollowSymbolUnderCursor_virtualFunctionCall_notOnDefinition()
{
    const QByteArray source =
            "struct A { virtual void virt(); };\n"
            "void A::virt() {}\n"
            "\n"
            "struct B : public A { void $virt(); };\n"
            "void B::@virt() {}\n"
            ;

    TestCase test(TestCase::FollowSymbolUnderCursorAction, source);
    test.run();
}

void CppEditorPlugin::test_FollowSymbolUnderCursor_virtualFunctionCall_notOnNonPointerNonReference()
{
    const QByteArray source =
            "struct A { virtual void virt(); };\n"
            "void A::virt() {}\n"
            "\n"
            "struct B : public A { void virt(); };\n"
            "void B::$virt() {}\n"
            "\n"
            "void client(B b) { b.@virt(); }\n"
            ;

    TestCase test(TestCase::FollowSymbolUnderCursorAction, source);
    test.run();
}

/*
Potential test cases improving name lookup.

If you fix one, add a test and remove the example from here.

///////////////////////////////////////////////////////////////////////////////////////////////////

// 3.3.1 Declarative regions and scopes, copy-paste (main added)
int j = 24;
int main()
{
    int i = j, j; // First j refers to global j, second j refers to just locally defined j
    j = 42; // Refers to locally defined j
}

///////////////////////////////////////////////////////////////////////////////////////////////////

// 3.3.2 Point of declaration (par. 2), copy-paste (main added)
const int i = 2;
int main()
{
    int i[i]; // Second i refers to global
}

///////////////////////////////////////////////////////////////////////////////////////////////////

// 3.3.2 Point of declaration (par. 9), copy-paste (main added)
typedef unsigned char T;
template<class T
= T // lookup finds the typedef name of unsigned char
, T // lookup finds the template parameter
N = 0> struct A { };

int main() {}

///////////////////////////////////////////////////////////////////////////////////////////////////

// 3.3.9 Template parameter scope (par. 3), copy-paste (main added), part 1
template<class T, T* p, class U = T> class X {}; // second and third T refers to first one
template<class T> void f(T* p = new T);

int main() {}

///////////////////////////////////////////////////////////////////////////////////////////////////

// 3.3.9 Template parameter scope (par. 3), copy-paste (main added), part 2
template<class T> class X : public Array<T> {};
template<class T> class Y : public T {};

int main() {}

///////////////////////////////////////////////////////////////////////////////////////////////////

// 3.3.9 Template parameter scope (par. 4), copy-paste (main added), part 2
typedef int N;
template<N X, typename N, template<N Y> class T> struct A; // N refers to N above

int main() {}

///////////////////////////////////////////////////////////////////////////////////////////////////

// 3.3.10 Name hiding (par 1.), from text, example 2a

// www.stroustrup.com/bs_faq2.html#overloadderived
// "In C++, there is no overloading across scopes - derived class scopes are not
// an exception to this general rule. (See D&E or TC++PL3 for details)."
struct B {
    int f(int) {}
};

struct D : public B {
    double f(double) {} // hides B::f
};

int main()
{
    D* pd = new D;
    pd->f(2); // refers to D::f
    pd->f(2.3); // refers to D::f
}

///////////////////////////////////////////////////////////////////////////////////////////////////

// 3.3.10 Name hiding (par 2.), from text
int C; // hides following type C, order is not important
struct C {};

int main()
{
    ++C;
}

*/
