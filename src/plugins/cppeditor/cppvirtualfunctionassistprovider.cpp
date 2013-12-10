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


#include "cppvirtualfunctionassistprovider.h"

#include "cppeditorconstants.h"
#include "cppelementevaluator.h"
#include "cppvirtualfunctionproposalitem.h"

#include <cplusplus/Icons.h>
#include <cplusplus/Overview.h>

#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>

#include <cpptools/symbolfinder.h>

#include <texteditor/codeassist/basicproposalitemlistmodel.h>
#include <texteditor/codeassist/genericproposal.h>
#include <texteditor/codeassist/genericproposalwidget.h>
#include <texteditor/codeassist/iassistinterface.h>
#include <texteditor/codeassist/iassistprocessor.h>
#include <texteditor/codeassist/iassistproposal.h>

#include <utils/qtcassert.h>

using namespace CPlusPlus;
using namespace CppEditor::Internal;
using namespace TextEditor;

/// Activate current item with the same shortcut that is configured for Follow Symbol Under Cursor.
/// This is limited to single-key shortcuts without modifiers.
class VirtualFunctionProposalWidget : public GenericProposalWidget
{
public:
    VirtualFunctionProposalWidget(bool openInSplit)
    {
        const char *id = openInSplit
            ? TextEditor::Constants::FOLLOW_SYMBOL_UNDER_CURSOR_IN_NEXT_SPLIT
            : TextEditor::Constants::FOLLOW_SYMBOL_UNDER_CURSOR;
        if (Core::Command *command = Core::ActionManager::command(id))
            m_sequence = command->keySequence();
    }

protected:
    bool eventFilter(QObject *o, QEvent *e)
    {
        if (e->type() == QEvent::ShortcutOverride && m_sequence.count() == 1) {
            QKeyEvent *ke = static_cast<QKeyEvent *>(e);
            const QKeySequence seq(ke->key());
            if (seq == m_sequence) {
                activateCurrentProposalItem();
                e->accept();
                return true;
            }
        }
        return GenericProposalWidget::eventFilter(o, e);
    }

private:
    QKeySequence m_sequence;
};

class VirtualFunctionProposal : public GenericProposal
{
public:
    VirtualFunctionProposal(int cursorPos, IGenericProposalModel *model, bool openInSplit)
        : GenericProposal(cursorPos, model)
        , m_openInSplit(openInSplit)
    {}

    bool isFragile() const
    { return true; }

    IAssistProposalWidget *createWidget() const
    { return new VirtualFunctionProposalWidget(m_openInSplit); }

private:
    bool m_openInSplit;
};

class VirtualFunctionsAssistProcessor : public IAssistProcessor
{
public:
    VirtualFunctionsAssistProcessor(const VirtualFunctionAssistProvider::Parameters &params)
        : m_params(params)
    {}

    IAssistProposal *immediateProposal(const TextEditor::IAssistInterface *)
    {
        QTC_ASSERT(m_params.function, return 0);

        BasicProposalItem *hintItem = new VirtualFunctionProposalItem(CPPEditorWidget::Link());
        hintItem->setText(QCoreApplication::translate("VirtualFunctionsAssistProcessor",
                                                      "...searching overrides"));
        hintItem->setOrder(-1000);

        QList<BasicProposalItem *> items;
        items << itemFromSymbol(maybeDefinitionFor(m_params.function));
        items << hintItem;
        return new VirtualFunctionProposal(m_params.cursorPosition,
                                           new BasicProposalItemListModel(items),
                                           m_params.openInNextSplit);
    }

    IAssistProposal *perform(const IAssistInterface *)
    {
        QTC_ASSERT(m_params.function, return 0);
        QTC_ASSERT(m_params.staticClass, return 0);
        QTC_ASSERT(!m_params.snapshot.isEmpty(), return 0);

        Class *functionsClass = m_finder.findMatchingClassDeclaration(m_params.function,
                                                                      m_params.snapshot);
        if (!functionsClass)
            return 0;

        const QList<Symbol *> overrides = FunctionHelper::overrides(
            m_params.function, functionsClass, m_params.staticClass, m_params.snapshot);
        if (overrides.isEmpty())
            return 0;

        QList<BasicProposalItem *> items;
        foreach (Symbol *symbol, overrides)
            items << itemFromSymbol(maybeDefinitionFor(symbol));
        items.first()->setOrder(1000); // Ensure top position for function of static type

        return new VirtualFunctionProposal(m_params.cursorPosition,
                                           new BasicProposalItemListModel(items),
                                           m_params.openInNextSplit);
    }

private:
    Symbol *maybeDefinitionFor(Symbol *symbol)
    {
        if (Function *definition = m_finder.findMatchingDefinition(symbol, m_params.snapshot))
            return definition;
        return symbol;
    }

    BasicProposalItem *itemFromSymbol(Symbol *symbol) const
    {
        const QString text = m_overview.prettyName(LookupContext::fullyQualifiedName(symbol));
        const CPPEditorWidget::Link link = CPPEditorWidget::linkToSymbol(symbol);

        BasicProposalItem *item = new VirtualFunctionProposalItem(link, m_params.openInNextSplit);
        item->setText(text);
        item->setIcon(m_icons.iconForSymbol(symbol));

        return item;
    }

    VirtualFunctionAssistProvider::Parameters m_params;
    Overview m_overview;
    Icons m_icons;
    CppTools::SymbolFinder m_finder;
};

VirtualFunctionAssistProvider::VirtualFunctionAssistProvider()
{
}

bool VirtualFunctionAssistProvider::configure(const Parameters &parameters)
{
    m_params = parameters;
    return true;
}

bool VirtualFunctionAssistProvider::isAsynchronous() const
{
    return true;
}

bool VirtualFunctionAssistProvider::supportsEditor(const Core::Id &editorId) const
{
    return editorId == CppEditor::Constants::CPPEDITOR_ID;
}

IAssistProcessor *VirtualFunctionAssistProvider::createProcessor() const
{
    return new VirtualFunctionsAssistProcessor(m_params);
}

enum VirtualType { Virtual, PureVirtual };

static bool isVirtualFunction_helper(const Function *function,
                                     const LookupContext &context,
                                     VirtualType virtualType,
                                     const Function **firstVirtual)
{
    enum { Unknown, False, True } res = Unknown;

    if (firstVirtual)
        *firstVirtual = 0;

    if (!function)
        return false;

    if (virtualType == PureVirtual)
        res = function->isPureVirtual() ? True : False;

    if (function->isVirtual()) {
        if (firstVirtual)
            *firstVirtual = function;
        if (res == Unknown)
            res = True;
    }

    if (!firstVirtual && res != Unknown)
        return res == True;

    QList<LookupItem> results = context.lookup(function->name(), function->enclosingScope());
    if (!results.isEmpty()) {
        const bool isDestructor = function->name()->isDestructorNameId();
        foreach (const LookupItem &item, results) {
            if (Symbol *symbol = item.declaration()) {
                if (Function *functionType = symbol->type()->asFunctionType()) {
                    if (functionType->name()->isDestructorNameId() != isDestructor)
                        continue;
                    if (functionType == function) // already tested
                        continue;
                    if (functionType->isFinal())
                        return res == True;
                    if (functionType->isVirtual()) {
                        if (!firstVirtual)
                            return true;
                        if (res == Unknown)
                            res = True;
                        *firstVirtual = functionType;
                    }
                }
            }
        }
    }

    return res == True;
}

bool FunctionHelper::isVirtualFunction(const Function *function,
                                       const LookupContext &context,
                                       const Function **firstVirtual)
{
    return isVirtualFunction_helper(function, context, Virtual, firstVirtual);
}

bool FunctionHelper::isPureVirtualFunction(const Function *function,
                                           const LookupContext &context,
                                           const Function **firstVirtual)
{
    return isVirtualFunction_helper(function, context, PureVirtual, firstVirtual);
}

QList<Symbol *> FunctionHelper::overrides(Function *function, Class *functionsClass,
                                          Class *staticClass, const Snapshot &snapshot)
{
    QList<Symbol *> result;
    QTC_ASSERT(function && functionsClass && staticClass, return result);

    FullySpecifiedType referenceType = function->type();
    const Name *referenceName = function->name();
    QTC_ASSERT(referenceName && referenceType.isValid(), return result);

    // Find overrides
    CppEditor::Internal::CppClass cppClass = CppClass(functionsClass);
    cppClass.lookupDerived(staticClass, snapshot);

    QList<CppClass> l;
    l << cppClass;

    while (!l.isEmpty()) {
        // Add derived
        CppClass clazz = l.takeFirst();

        QTC_ASSERT(clazz.declaration, continue);
        Class *c = clazz.declaration->asClass();
        QTC_ASSERT(c, continue);

        foreach (const CppClass &d, clazz.derived) {
            if (!l.contains(d))
                l << d;
        }

        // Check member functions
        for (int i = 0, total = c->memberCount(); i < total; ++i) {
            Symbol *candidate = c->memberAt(i);
            const Name *candidateName = candidate->name();
            const FullySpecifiedType candidateType = candidate->type();
            if (!candidateName || !candidateType.isValid())
                continue;
            if (candidateName->isEqualTo(referenceName) && candidateType.isEqualTo(referenceType))
                result << candidate;
        }
    }

    return result;
}

#ifdef WITH_TESTS
#include "cppeditorplugin.h"

#include <QList>
#include <QTest>

namespace CppEditor {
namespace Internal {

enum Virtuality
{
    NotVirtual,
    Virtual,
    PureVirtual
};
typedef QList<Virtuality> VirtualityList;
} // Internal namespace
} // CppEditor namespace

Q_DECLARE_METATYPE(CppEditor::Internal::Virtuality)
Q_DECLARE_METATYPE(CppEditor::Internal::VirtualityList)
Q_DECLARE_METATYPE(QList<int>)

namespace CppEditor {
namespace Internal {

void CppEditorPlugin::test_functionhelper_virtualFunctions()
{
    // Create and parse document
    QFETCH(QByteArray, source);
    QFETCH(VirtualityList, virtualityList);
    QFETCH(QList<int>, firstVirtualList);
    Document::Ptr document = Document::create(QLatin1String("virtuals"));
    document->setUtf8Source(source);
    document->check(); // calls parse();
    QCOMPARE(document->diagnosticMessages().size(), 0);
    QVERIFY(document->translationUnit()->ast());
    QList<const Function *> allFunctions;
    const Function *firstVirtual = 0;

    // Iterate through Function symbols
    Snapshot snapshot;
    snapshot.insert(document);
    const LookupContext context(document, snapshot);
    Control *control = document->translationUnit()->control();
    Symbol **end = control->lastSymbol();
    for (Symbol **it = control->firstSymbol(); it != end; ++it) {
        if (const Function *function = (*it)->asFunction()) {
            allFunctions.append(function);
            QTC_ASSERT(!virtualityList.isEmpty(), return);
            Virtuality virtuality = virtualityList.takeFirst();
            QTC_ASSERT(!firstVirtualList.isEmpty(), return);
            int firstVirtualIndex = firstVirtualList.takeFirst();
            bool isVirtual = FunctionHelper::isVirtualFunction(function, context, &firstVirtual);
            bool isPureVirtual = FunctionHelper::isPureVirtualFunction(function, context,
                                                                       &firstVirtual);

            // Test for regressions introduced by firstVirtual
            QCOMPARE(FunctionHelper::isVirtualFunction(function, context), isVirtual);
            QCOMPARE(FunctionHelper::isPureVirtualFunction(function, context), isPureVirtual);
            if (isVirtual) {
                if (isPureVirtual)
                    QCOMPARE(virtuality, PureVirtual);
                else
                    QCOMPARE(virtuality, Virtual);
            } else {
                QEXPECT_FAIL("virtual-dtor-dtor", "Not implemented", Abort);
                if (allFunctions.size() == 3)
                    QEXPECT_FAIL("dtor-virtual-dtor-dtor", "Not implemented", Abort);
                QCOMPARE(virtuality, NotVirtual);
            }
            if (firstVirtualIndex == -1)
                QVERIFY(!firstVirtual);
            else
                QCOMPARE(firstVirtual, allFunctions.at(firstVirtualIndex));
        }
    }
    QVERIFY(virtualityList.isEmpty());
    QVERIFY(firstVirtualList.isEmpty());
}

void CppEditorPlugin::test_functionhelper_virtualFunctions_data()
{
    typedef QByteArray _;
    QTest::addColumn<QByteArray>("source");
    QTest::addColumn<VirtualityList>("virtualityList");
    QTest::addColumn<QList<int> >("firstVirtualList");

    QTest::newRow("none")
            << _("struct None { void foo() {} };\n")
            << (VirtualityList() << NotVirtual)
            << (QList<int>() << -1);

    QTest::newRow("single-virtual")
            << _("struct V { virtual void foo() {} };\n")
            << (VirtualityList() << Virtual)
            << (QList<int>() << 0);

    QTest::newRow("single-pure-virtual")
            << _("struct PV { virtual void foo() = 0; };\n")
            << (VirtualityList() << PureVirtual)
            << (QList<int>() << 0);

    QTest::newRow("virtual-derived-with-specifier")
            << _("struct Base { virtual void foo() {} };\n"
                 "struct Derived : Base { virtual void foo() {} };\n")
            << (VirtualityList() << Virtual << Virtual)
            << (QList<int>() << 0 << 0);

    QTest::newRow("virtual-derived-implicit")
            << _("struct Base { virtual void foo() {} };\n"
                 "struct Derived : Base { void foo() {} };\n")
            << (VirtualityList() << Virtual << Virtual)
            << (QList<int>() << 0 << 0);

    QTest::newRow("not-virtual-then-virtual")
            << _("struct Base { void foo() {} };\n"
                 "struct Derived : Base { virtual void foo() {} };\n")
            << (VirtualityList() << NotVirtual << Virtual)
            << (QList<int>() << -1 << 1);

    QTest::newRow("virtual-final-not-virtual")
            << _("struct Base { virtual void foo() {} };\n"
                 "struct Derived : Base { void foo() final {} };\n"
                 "struct Derived2 : Derived { void foo() {} };")
            << (VirtualityList() << Virtual << Virtual << NotVirtual)
            << (QList<int>() << 0 << 0 << -1);

    QTest::newRow("virtual-then-pure")
            << _("struct Base { virtual void foo() {} };\n"
                 "struct Derived : Base { virtual void foo() = 0; };\n"
                 "struct Derived2 : Derived { void foo() {} };")
            << (VirtualityList() << Virtual << PureVirtual << Virtual)
            << (QList<int>() << 0 << 0 << 0);

    QTest::newRow("virtual-virtual-final-not-virtual")
            << _("struct Base { virtual void foo() {} };\n"
                 "struct Derived : Base { virtual void foo() final {} };\n"
                 "struct Derived2 : Derived { void foo() {} };")
            << (VirtualityList() << Virtual << Virtual << NotVirtual)
            << (QList<int>() << 0 << 0 << -1);

    QTest::newRow("ctor-virtual-dtor")
            << _("struct Base { Base() {} virtual ~Base() {} };\n")
            << (VirtualityList() << NotVirtual << Virtual)
            << (QList<int>() << -1 << 1);

    QTest::newRow("virtual-dtor-dtor")
            << _("struct Base { virtual ~Base() {} };\n"
                 "struct Derived : Base { ~Derived() {} };\n")
            << (VirtualityList() << Virtual << Virtual)
            << (QList<int>() << 0 << 0);

    QTest::newRow("dtor-virtual-dtor-dtor")
            << _("struct Base { ~Base() {} };\n"
                 "struct Derived : Base { virtual ~Derived() {} };\n"
                 "struct Derived2 : Derived { ~Derived2() {} };\n")
            << (VirtualityList() << NotVirtual << Virtual << Virtual)
            << (QList<int>() << -1 << 1 << 1);
}

} // namespace Internal
} // namespace CppEditor

#endif
