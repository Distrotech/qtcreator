source("../../shared/qtcreator.py")
import re

SpeedCrunchPath = ""

def buildConfigFromFancyToolButton(fancyToolButton):
    beginOfBuildConfig = "<b>Build:</b> "
    endOfBuildConfig = "<br/><b>Deploy:</b>"
    toolTipText = str(fancyToolButton.toolTip)
    beginIndex = toolTipText.find(beginOfBuildConfig) + len(beginOfBuildConfig)
    endIndex = toolTipText.find(endOfBuildConfig)
    return toolTipText[beginIndex:endIndex]

def main():
    if not neededFilePresent(SpeedCrunchPath):
        return
    startApplication("qtcreator" + SettingsPath)
    openQmakeProject(SpeedCrunchPath)
    waitForSignal("{type='CppTools::Internal::CppModelManager' unnamed='1'}", "sourceFilesRefreshed(QStringList)")

    # Test that some of the expected items are in the navigation tree
    for row, record in enumerate(testData.dataset("speedcrunch_tree.tsv")):
        node = testData.field(record, "node")
        value = testData.field(record, "value")
        test.compare(waitForObject(node).text, value)

    fancyToolButton = waitForObject(":*Qt Creator_Core::Internal::FancyToolButton")

    qtVerPattern = re.compile("\d\.\d(\.\d+)?")
    for config in iterateBuildConfigs(1, 0, "(Desktop )?Qt.*Release"):
        qtVersion = qtVerPattern.search(config)
        if qtVersion:
            qtVersion = qtVersion.group()
            if qtVersion >= "4.8":
                test.log("Skipping config %s - this project needs Qt <= 4.7.x (got %s)" % (config, qtVersion))
                continue
        else:
            test.warning("Could not determine Qt version for config '%s' - skipping..." % config)
            continue
        selectBuildConfig(1, 0, config)
        buildConfig = buildConfigFromFancyToolButton(fancyToolButton)
        if buildConfig != config:
            test.fatal("Build configuration %s is selected instead of %s" % (buildConfig, config))
            continue
        test.log("Testing build configuration: " + config)
        invokeMenuItem("Build", "Run qmake")
        waitForSignal("{type='ProjectExplorer::BuildManager' unnamed='1'}", "buildQueueFinished(bool)")
        invokeMenuItem("Build", "Rebuild All")
        waitForSignal("{type='ProjectExplorer::BuildManager' unnamed='1'}", "buildQueueFinished(bool)", 300000)
        checkCompile()
        checkLastBuild()

    # Add a new run configuration

    invokeMenuItem("File", "Exit")
    waitForCleanShutdown()

def init():
    global SpeedCrunchPath
    SpeedCrunchPath = srcPath + "/creator-test-data/speedcrunch/src/speedcrunch.pro"
    cleanup()

def cleanup():
    # Make sure the .user files are gone
    cleanUpUserFiles(SpeedCrunchPath)

    BuildPath = glob.glob(srcPath + "/creator-test-data/speedcrunch/speedcrunch-build-*")
    BuildPath += glob.glob(srcPath + "/creator-test-data/speedcrunch/qtcreator-build-*")

    if BuildPath:
        for dir in BuildPath:
            if os.access(dir, os.F_OK):
                shutil.rmtree(dir)
