source("../../shared/qtcreator.py")

import re

def main():
    global templateDir, textChanged
    sourceExample = os.path.abspath(sdkPath + "/Examples/4.7/declarative/text/textselection")
    if not neededFilePresent(sourceExample):
        return
    templateDir = prepareTemplate(sourceExample)
    startApplication("qtcreator" + SettingsPath)
    overrideInstallLazySignalHandler()
    installLazySignalHandler(":frame.templateDescription_QTextBrowser",
                             "textChanged()","__handleTextChanged__")
    targets = getCorrectlyConfiguredTargets()
    test.log("Collecting potential project types...")
    availableProjectTypes = []
    invokeMenuItem("File", "New File or Project...")
    categoriesView = waitForObject(":New.templateCategoryView_QTreeView", 20000)
    catModel = categoriesView.model()
    projects = catModel.index(0, 0)
    test.compare("Projects", str(projects.data()))
    comboBox = waitForObject("{name='comboBox' type='QComboBox' visible='1' "
                             "window=':New_Core::Internal::NewDialog'}")
    test.compare(comboBox.currentText, "All Templates")
    for row in range(catModel.rowCount(projects)):
        index = catModel.index(row, 0, projects)
        category = str(index.data()).replace(".", "\\.")
        # skip non-configurable
        if "Import" in category or "Non-Qt" in category:
            continue
        clickItem(categoriesView, "Projects." + category, 5, 5, 0, Qt.LeftButton)
        templatesView = waitForObject("{name='templatesView' type='QListView' visible='1'}", 20000)
        # needed because categoriesView and templatesView using same model
        tempRootIndex = templatesView.rootIndex()
        tempModel = templatesView.model()
        for tRow in range(tempModel.rowCount(tempRootIndex)):
            tIndex = tempModel.index(tRow, 0, tempRootIndex)
            template = str(tempModel.data(tIndex)).replace(".", "\\.")
            # skip non-configurable
            if "Qt Quick UI" in template or "Plain C" in template:
                continue
            availableProjectTypes.append({category:template})
    clickButton(waitForObject("{text='Cancel' type='QPushButton' unnamed='1' visible='1'}", 20000))
    for current in availableProjectTypes:
        category = current.keys()[0]
        template = current.values()[0]
        invokeMenuItem("File", "New File or Project...")
        categoriesView = waitForObject(":New.templateCategoryView_QTreeView", 20000)
        clickItem(categoriesView, "Projects." + category, 5, 5, 0, Qt.LeftButton)
        templatesView = waitForObject("{name='templatesView' type='QListView' visible='1'}", 20000)
        test.log("Verifying '%s' -> '%s'" % (category.replace("\\.", "."), template.replace("\\.", ".")))
        textChanged = False
        clickItem(templatesView, template, 5, 5, 0, Qt.LeftButton)
        waitFor("textChanged", 2000)
        text = waitForObject(":frame.templateDescription_QTextBrowser").plainText
        displayedPlatforms, requiredVersion = __getSupportedPlatforms__(str(text), True)
        clickButton(waitForObject("{text='Choose...' type='QPushButton' unnamed='1' visible='1'}", 20000))
        # don't check because project could exist
        __createProjectSetNameAndPath__(os.path.expanduser("~"), 'untitled', False)
        try:
            waitForObject("{name='mainQmlFileGroupBox' title='Main HTML File' type='QGroupBox' visible='1'}", 1000)
            clickButton(waitForObject(":Next_QPushButton"))
        except LookupError:
            try:
                waitForObject("{text='Select Existing QML file' type='QLabel' visible='1'}", 1000)
                baseLineEd = waitForObject("{type='Utils::BaseValidatingLineEdit' unnamed='1' visible='1'}", 20000)
                type(baseLineEd, templateDir + "/qml/textselection.qml")
                clickButton(waitForObject(":Next_QPushButton"))
            except LookupError:
                pass
        waitForObject("{type='QLabel' unnamed='1' visible='1' text='Target Setup'}")
        availableCheckboxes = filter(visibleCheckBoxExists, QtQuickConstants.getAllTargetStrings())
        JIRA.performWorkaroundIfStillOpen(6994, JIRA.Bug.CREATOR, template, displayedPlatforms)
        # verification whether expected, found and configured match
        for t in targets:
            if requiredVersion:
                if max(targets[t]) < requiredVersion:
                    if t in availableCheckboxes:
                        test.fail("Target '%s' found as checkbox, but required version (%s) is higher "
                                  "than configured version(s) (%s)!" % (t, requiredVersion, str(targets[t])))
                        availableCheckboxes.remove(t)
                    else:
                        test.passes("Irrelevant target '%s' not found on 'Target setup' page - "
                                    "required version is '%s', current version(s) are '%s'." %
                                    (t, requiredVersion, str(targets[t])))
                    continue
            found = False
            if t in displayedPlatforms:
                if t in availableCheckboxes:
                    test.passes("Found expected target '%s' on 'Target setup' page." % t)
                    availableCheckboxes.remove(t)
                else:
                    test.fail("Expected target '%s' missing on 'Target setup' page." % t)
            else:
                if t in availableCheckboxes:
                    test.fail("Target '%s' found on 'Target setup' page - but has not been expected!" % t)
                else:
                    test.passes("Irrelevant target '%s' not found on 'Target setup' page." % t)
        if len(availableCheckboxes) != 0:
            test.fail("Found unexpected additional target(s) %s on 'Target setup' page." % str(availableCheckboxes))
        clickButton(waitForObject("{text='Cancel' type='QPushButton' unnamed='1' visible='1'}", 20000))
    invokeMenuItem("File", "Exit")

def cleanup():
    global templateDir
    # waiting for a clean exit - for a full-remove of the temp directory
    waitForCleanShutdown()
    if templateDir!=None:
        deleteDirIfExists(os.path.dirname(templateDir))

def __handleTextChanged__(object):
    global textChanged
    textChanged = True
