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

import QtQuick 1.1

Item {
    id: toolTip

    property alias text: text.text


    property int margin: 4

    width: text.width + margin * 2
    height: text.height + margin * 2

    opacity: 0

    property Item originalParent: parent

    property int oldX: x
    property int oldY: y

    Behavior on opacity {
        SequentialAnimation {
            PauseAnimation { duration: opacity === 0 ? 1000 : 0 }
            PropertyAnimation { duration: 50 }
        }
    }

    function show() {
        toolTip.originalParent = toolTip.parent;
        var p = toolTip.parent;
        while (p.parent != undefined && p.parent.parent != undefined)
            p = p.parent
        toolTip.parent = p;

        toolTip.oldX = toolTip.x
        toolTip.oldY = toolTip.y
        var globalPos = mapFromItem(toolTip.originalParent, toolTip.oldX, toolTip.oldY);

        toolTip.x = globalPos.x + toolTip.oldX
        toolTip.y = globalPos.y + toolTip.oldY

        toolTip.opacity = 1;
    }

    function hide() {
        toolTip.opacity = 0;
        var oldClip = originalParent.clip
        originalParent.clip = false
        toolTip.parent = originalParent
        originalParent.clip = true
        originalParent.clip = oldClip
        toolTip.x = toolTip.oldX
        toolTip.y = toolTip.oldY
    }

    Rectangle {
        anchors.fill: parent

        border.width: 1
        smooth: true
        radius: 2
        gradient: Gradient {
            GradientStop {
                position: 0.00;
                color: "#ffffff";
            }
            GradientStop {
                position: 1.00;
                color: "#e4e5f0";
            }
        }
    }

    Text {
        x: toolTip.margin
        y: toolTip.margin
        id: text
    }
}
