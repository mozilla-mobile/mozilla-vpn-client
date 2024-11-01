/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import components 0.1
import compat 0.1

Rectangle {
    id: root
    objectName: "navigationBar"

    height: MZTheme.theme.navBarHeight
    width: Math.min(window.width - MZTheme.theme.windowMargin * 2, MZTheme.theme.navBarMaxWidth)
    radius: height / 2
    color: MZTheme.colors.ink

    anchors {
        horizontalCenter: parent.horizontalCenter
        bottom: parent.bottom
        bottomMargin: MZTheme.theme.navBarBottomMargin
    }

    MZDropShadow {
        source: outline
        anchors.fill: outline
        transparentBorder: true
        verticalOffset: 2
        opacity: 0.6
        spread: 0
        radius: 6
        color: MZTheme.colors.grey60
        cached: true
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
    }

    Rectangle {
        id: outline
        color: MZTheme.colors.ink
        radius: parent.radius
        anchors.fill: parent
        border.width: 0
        border.color: MZTheme.colors.ink
    }

    RowLayout {
        id: layout
        objectName: "navigationLayout"

        anchors.fill: parent
        anchors.topMargin: MZTheme.theme.vSpacingSmall / 2
        anchors.leftMargin: {
            let minNumberOfIcons = 3
            let paddingFactor = 8 //How much we decrease horizontal margins by for each new icon added (for non-tablets)
            window.width < MZTheme.theme.tabletMinimumWidth ? MZTheme.theme.navBarMaxPadding - (paddingFactor * (MZNavigationBarModel.count - minNumberOfIcons)) : MZTheme.theme.navBarMaxPaddingTablet
        }
        anchors.rightMargin: anchors.leftMargin
        anchors.bottomMargin: MZTheme.theme.vSpacingSmall / 2

        spacing: (root.width - anchors.leftMargin - anchors.rightMargin - (MZTheme.theme.navBarIconSize * MZNavigationBarModel.count)) / (MZNavigationBarModel.count - 1)

        Repeater {
            model: MZNavigationBarModel
            delegate: MZBottomNavigationBarButton {
                objectName: buttonItem.objectName

                Layout.preferredHeight: height
                Layout.preferredWidth: width

                skipEnsureVisible: true
                _screen: buttonItem.screen
                _source: buttonItem.source

                accessibleName: MZI18n[buttonItem.navAccessibleName]
                enabled: root.visible

                checked: buttonItem.checked
            }
        }
    }
}
