/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import components 0.1

MZIconButton {
    id: btn

    property var _screen
    property alias _source: image.source
    property bool _hasNotification: false

    accessibleName: ""

    onClicked: {
        if (MZNavigator.screen === _screen) {
            window.unwindStackView();
            return;
        }

        MZNavigator.requestScreenFromBottomBar(_screen, MZNavigator.NoFlags);
    }

    width: MZTheme.theme.navBarIconSize
    height: MZTheme.theme.navBarIconSize

    backgroundRadius: height / 2
    buttonColorScheme: MZTheme.colors.iconButtonDarkBackground
    uiStatesVisible: !checked

    Rectangle {
        id: circleBackground
        anchors.fill: parent
        visible: checked
        radius: parent.height / 2
        opacity: .2
        color: MZTheme.colors.white
    }

    Image {
        id: image
        anchors.centerIn: parent
        sourceSize.height: MZTheme.theme.iconSize * 2
        sourceSize.width: MZTheme.theme.iconSize * 2
    }
}
