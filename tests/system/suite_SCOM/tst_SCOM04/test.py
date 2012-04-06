source("../../shared/qtcreator.py")
source("../../shared/suites_qtta.py")

# entry of test
def main():
    # expected error texts - for different compilers
    expectedErrorAlternatives = ["'SyntaxError' was not declared in this scope",
                                 "'SyntaxError' : undeclared identifier"]
    startApplication("qtcreator" + SettingsPath)
    # create qt quick application
    createNewQtQuickApplication(tempDir(), "SampleApp")
    # create syntax error in cpp file
    doubleClickItem(":Qt Creator_Utils::NavigationTreeView", "SampleApp.Sources.main\\.cpp", 5, 5, 0, Qt.LeftButton)
    if not appendToLine(waitForObject(":Qt Creator_CppEditor::Internal::CPPEditorWidget"), "viewer.showExpanded();", "SyntaxError"):
        invokeMenuItem("File", "Exit")
        return
    # save all
    invokeMenuItem("File", "Save All")
    # build it - on all (except Qt 4.7.0 (would fail)) build configurations
    for config in iterateBuildConfigs(1, 0, "(?!.*4\.7\.0.*)"):
        selectBuildConfig(1, 0, config)
        # try to compile
        test.log("Testing build configuration: " + config)
        clickButton(waitForObject(":Qt Creator.Build Project_Core::Internal::FancyToolButton"))
        # wait until build finished
        waitForSignal("{type='ProjectExplorer::BuildManager' unnamed='1'}", "buildQueueFinished(bool)")
        # open issues list view
        ensureChecked(waitForObject(":Qt Creator_Core::Internal::IssuesPaneToggleButton"))
        issuesView = waitForObject(":Qt Creator.Issues_QListView")
        # verify that error is properly reported
        test.verify(checkSyntaxError(issuesView, expectedErrorAlternatives, False),
                    "Verifying cpp syntax error while building simple qt quick application.")
    # exit qt creator
    invokeMenuItem("File", "Exit")
# no cleanup needed, as whole testing directory gets properly removed after test finished

