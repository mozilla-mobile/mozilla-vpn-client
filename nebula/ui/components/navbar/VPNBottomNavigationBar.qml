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

        ListModel {
            id: navButtons
            ListElement {
                navObjectName: "navButton-home"
                screen: "ScreenHome"
                sourceChecked: "qrc:/nebula/resources/navbar/home-selected.svg"
                sourceUnchecked: "qrc:/nebula/resources/navbar/home.svg"
                _accessibleName:"NavBarHomeTab"
            }
            ListElement {
                navObjectName: "navButton-messages"
                screen: "ScreenMessaging"
                sourceChecked: "qrc:/nebula/resources/navbar/messages-notification-selected.svg"
                sourceUnchecked: "qrc:/nebula/resources/navbar/messages-notification.svg"
                _accessibleName: "NavBarMessagesTab"
            }
            ListElement {
                navObjectName: "navButton-settings"
                screen: "ScreenSettings"
                sourceChecked: "qrc:/nebula/resources/navbar/settings-selected.svg"
                sourceUnchecked: "qrc:/nebula/resources/navbar/settings.svg"
                _accessibleName: "NavBarSettingsTab"
            }
        }

        RowLayout {
            id: buttonRow
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
                        accessibleName: VPNl18n[_accessibleName]
                        ButtonGroup.group: navBarButtonGroup
                        Component.onCompleted: if (index === 0) checked = true;
                    }
                }
            }
        }
    }

    states: [
        State {
            when: VPN.state !== VPN.StateMain
            PropertyChanges {
                target: root
                visible: false
            }
        },
        State {
            when:
                VPN.state === VPN.StateMain &&
                [
                    VPNNavigator.ScreenSettings,
                    VPNNavigator.ScreenHome,
                    VPNNavigator.ScreenMessaging,
                    VPNNavigator.ScreenGetHelp
                ].includes(VPNNavigator.screen)
            PropertyChanges {
                target: root
                visible: true
            }
        }
    ]

    ButtonGroup {
        id: navBarButtonGroup
    }
}
