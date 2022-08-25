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
    id: root
    objectName: "navigationBar"

    height: VPNTheme.theme.navBarHeight
    width: Math.min(window.width - VPNTheme.theme.windowMargin * 2, VPNTheme.theme.navBarMaxWidth)
    radius: height / 2
    color: VPNTheme.theme.ink
    clip: true

    anchors {
        horizontalCenter: parent.horizontalCenter
        bottom: parent.bottom
        bottomMargin: VPNTheme.theme.navBarBottomMargin
    }

    Flickable{
        id: flickable
        clip: true
        anchors.fill: root
        contentWidth: layout.implicitWidth
        interactive: contentX > 0 || contentWidth > width

        RowLayout {
            id: buttonRow
            height: flickable.height
            width: flickable.width

            RowLayout {
                id: layout
                Layout.alignment: Qt.AlignHCenter
                spacing: flickable.width * .2 // TODO something better here
                data: [
                    VPNBottomNavigationBarButton {
                        id: homeNavButton
                        source: checked ? "qrc:/nebula/resources/navbar/home-selected.svg" : "qrc:/nebula/resources/navbar/home.svg"
                        ButtonGroup.group: navBarButtonGroup
                        checked: true
                        accessibleName: VPNl18n["NavBarHomeTab"]
                        onClicked: {
                            // TODO... check current screen and stackview depth
                            // maybe unwind stack to initial view
                            VPNNavigator.requestScreen(VPNNavigator.ScreenHome)
                        }
                    },
                    VPNBottomNavigationBarButton {
                        id: messagesNavButton
                        source: checked ? (hasNotification ? "qrc:/nebula/resources/navbar/messages-notification-selected.svg" : "qrc:/nebula/resources/navbar/messages-selected.svg") : (hasNotification ? "qrc:/nebula/resources/navbar/messages-notification.svg" : "qrc:/nebula/resources/navbar/messages.svg")
                        ButtonGroup.group: navBarButtonGroup
                        accessibleName: VPNl18n["NavBarMessagesTab"]
                        onClicked: VPNNavigator.requestScreen(VPNNavigator.ScreenMessaging)
                    },
                    VPNBottomNavigationBarButton {
                        id: settingsNavButton
                        objectName: "settingsNavButton"
                        source: checked ? "qrc:/nebula/resources/navbar/settings-selected.svg" : "qrc:/nebula/resources/navbar/settings.svg"
                        ButtonGroup.group: navBarButtonGroup
                        accessibleName: VPNl18n["NavBarSettingsTab"]
                        onClicked: {
                            // TODO... check current screen and stackview depth
                            // maybe unwind stack to initial view
                            VPN.recordGleanEvent("settingsViewOpened");
                            VPNNavigator.requestScreen(VPNNavigator.ScreenSettings)
                        }
                    }
                ]
            }
        }
    }

    VPNDropShadow {
        id: shadow
        source: outline
        anchors.fill: outline
        transparentBorder: true
        verticalOffset: 2
        opacity: 0.6
        spread: 0
        radius: parent.radius
        color: VPNTheme.colors.grey60
        cached: true
        z: -1
    }

    Rectangle {
        id: outline
        color: VPNTheme.theme.ink
        radius: parent.radius
        anchors.fill: parent
        border.width: 0
        border.color: VPNTheme.theme.ink
        z: -1
    }


    // TODO add rules for nav bar visibility
    states: [
        State {
            when: VPN.state === VPN.StateMain
            PropertyChanges {
                target: root
                visible: true
            }
        },
        State {
            when: VPN.state !== VPN.StateMain
            PropertyChanges {
                target: root
                visible: false
            }
        }
    ]

    Behavior on opacity {
        PropertyAnimation {
            duration: 500
        }
    }

    ButtonGroup {
        id: navBarButtonGroup
    }
}
