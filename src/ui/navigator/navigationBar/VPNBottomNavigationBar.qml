/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import compat 0.1

Rectangle {
    property var showNavigationBar: [
        VPNNavigator.ScreenSettings,
        VPNNavigator.ScreenHome,
        VPNNavigator.ScreenMessaging,
        VPNNavigator.ScreenGetHelp
    ]

    id: root
    objectName: "navigationBar"

    height: VPNTheme.theme.navBarHeight
    width: Math.min(window.width - VPNTheme.theme.windowMargin * 2, VPNTheme.theme.navBarMaxWidth)
    radius: height / 2
    color: VPNTheme.theme.ink

    visible: showNavigationBar.includes(VPNNavigator.screen) && VPN.userState === VPN.UserAuthenticated

    anchors {
        horizontalCenter: parent.horizontalCenter
        bottom: parent.bottom
        bottomMargin: VPNTheme.theme.navBarBottomMargin
    }

    VPNDropShadow {
        source: outline
        anchors.fill: outline
        transparentBorder: true
        verticalOffset: 2
        opacity: 0.6
        spread: 0
        radius: parent.radius
        color: VPNTheme.colors.grey60
        cached: true
    }

    Rectangle {
        id: outline
        color: VPNTheme.theme.ink
        radius: parent.radius
        anchors.fill: parent
        border.width: 0
        border.color: VPNTheme.theme.ink
    }

    Flickable{
        id: flickable
        clip: true
        anchors.fill: root
        contentWidth: layout.implicitWidth
        interactive: contentX > 0 || contentWidth > width

        RowLayout {
            height: flickable.height
            width: flickable.width

            RowLayout {
                id: layout
                objectName: "navigationLayout"
                Layout.alignment: Qt.AlignHCenter
                spacing: flickable.width * .2 // TODO something better here

                Repeater {
                    model: navButtons
                    delegate: VPNBottomNavigationBarButton {
                        objectName: navObjectName
                        _screen: VPNNavigator[screen]
                        _source: checked ? sourceChecked : sourceUnchecked
                        accessibleName: VPNl18n[navAccessibleName]
                        ButtonGroup.group: navBarButtonGroup
                    }
                }
            }
        }
    }

    ListModel {
        id: navButtons
        ListElement {
            navObjectName: "navButton-home"
            screen: "ScreenHome"
            sourceChecked: "qrc:/nebula/resources/navbar/home-selected.svg"
            sourceUnchecked: "qrc:/nebula/resources/navbar/home.svg"
            navAccessibleName: "NavBarHomeTab"
        }
        ListElement {
            navObjectName: "navButton-messages"
            screen: "ScreenMessaging"
            sourceChecked: "qrc:/nebula/resources/navbar/messages-notification-selected.svg"
            sourceUnchecked: "qrc:/nebula/resources/navbar/messages-notification.svg"
            navAccessibleName: "NavBarMessagesTab"
        }
        ListElement {
            navObjectName: "navButton-settings"
            screen: "ScreenSettings"
            sourceChecked: "qrc:/nebula/resources/navbar/settings-selected.svg"
            sourceUnchecked: "qrc:/nebula/resources/navbar/settings.svg"
            navAccessibleName: "NavBarSettingsTab"
        }
    }

    ButtonGroup {
        id: navBarButtonGroup
    }
}
