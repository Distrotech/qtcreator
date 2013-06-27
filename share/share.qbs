import qbs.base 1.0

Product {
    name: "SharedContent"

    Group {
        name: "Unconditional"
        qbs.install: true
        qbs.installDir: project.ide_data_path
        prefix: "qtcreator/"
        files: [
            "designer",
            "dumper",
            "generic-highlighter",
            "glsl",
            "qml",
            "qmldesigner",
            "qmlicons",
            "qml-type-descriptions",
            "schemes",
            "snippets",
            "styles",
            "templates",
            "welcomescreen"
        ]
    }

    Group {
        name: "Conditional"
        qbs.install: true
        qbs.installDir: project.ide_data_path + "/externaltools"
        prefix: "../src/share/qtcreator/externaltools/"
        files: {
            var list = [
                "lrelease.xml",
                "lupdate.xml",
                "qmlviewer.xml",
                "sort.xml",
            ]
            switch (qbs.targetOS) {
            case "windows":
                list.push("notepad_win.xml");
                break;
            case "mac":
                list.push("vi_mac.xml");
                break;
            default:
                list.push("vi.xml");
            }
            return list;
        }
    }
}

