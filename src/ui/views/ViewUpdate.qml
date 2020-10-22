/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Mozilla.VPN 1.0
import "../components"
import "../themes/themes.js" as Theme

Flickable {
    id: updatePanel

    // Approximate height of components on load, written statically to avoid a binding loop
    property var childContentY: state === "recommended" ? 496 : 518
    property var windowHeightExceedsContentHeight: (window.height > childContentY)

    state: VPN.updateRecommended ? "recommended" : "required"
    anchors.fill: parent
    contentHeight: windowHeightExceedsContentHeight ? window.height : childContentY
    boundsBehavior: Flickable.StopAtBounds
    opacity: 0
    Component.onCompleted: {
        opacity = 1;
    }
    states: [
        State {
            name: "recommended"

            PropertyChanges {
                target: contentWrapper
                //% "Update recommended"
                logoTitle: qsTrId("vpn.updates.updateRecomended")
                //% "Please update the app before you continue to use the VPN"
                logoSubtitle: qsTrId("vpn.updates.updateRecomended.description")
                anchors.topMargin: 24
            }

            PropertyChanges {
                target: insetCircle
                color: Theme.blue
            }

            PropertyChanges {
                target: insetImage
                source: "../resources/down.svg"
            }

            PropertyChanges {
                target: signOff
                visible: false
            }

            PropertyChanges {
                target: footerLink
                onClicked: {
                    // TODO Should we hide the alert after "Not now" is clicked ?
                    // Can it be accessed again?
                    alertBox.visible = true;
                    stackview.pop(StackView.Immediate);
                }
                anchors.bottomMargin: 40
            }

        },
        State {
            name: "required"

            PropertyChanges {
                target: contentWrapper
                //% "Update required"
                logoTitle: qsTrId("vpn.updates.updateRequired")
                //% "We detected and fixed a serious bug. You must update your app."
                logoSubtitle: qsTrId("vpn.updates.updateRequire.reason")
                anchors.topMargin: 10
            }

            PropertyChanges {
                target: insetCircle
                color: Theme.red
            }

            PropertyChanges {
                target: insetImage
                source: "../resources/warning-white.svg"
            }

            PropertyChanges {
                target: signOff
                visible: VPN.userAuthenticated
            }

            PropertyChanges {
                target: footerLink
                anchors.bottom: signOff.top
                anchors.bottomMargin: 12
                onClicked: {
                    // TODO - should anything happen here besides opening
                    // the user account website?
                    VPN.openLink(VPN.LinkAccount);
                }
            }

        }
    ]

    VPNPanel {
        id: contentWrapper

        height: updatePanel.contentHeight - footerWrapper.height - anchors.topMargin
        anchors.top: parent.top
        width: parent.width
        logo: "../resources/update-lock.svg"

        Rectangle {
            id: insetCircle

            height: 42
            width: height
            radius: height / 2
            border.width: 5
            border.color: Theme.bgColor
            antialiasing: true
            anchors.left: contentWrapper.left
            anchors.leftMargin: contentWrapper.width / 2
            anchors.verticalCenter: parent.verticalCenter
            anchors.verticalCenterOffset: -36

            Image {
                id: insetImage

                anchors.centerIn: insetCircle
                sourceSize.height: 13
                fillMode: Image.PreserveAspectFit
            }

        }

    }

    Rectangle {
        id: footerWrapper

        property real footerContentHeight: alertWrapper.height + alertWrapper.anchors.bottomMargin + updateBtn.height + updateBtn.anchors.bottomMargin + footerLink.height + footerLink.anchors.bottomMargin

        height: (updatePanel.state === "recommended") ? footerContentHeight : footerContentHeight + signOff.height + signOff.anchors.bottomMargin
        width: parent.width
        color: "transparent"
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 0

        VPNDropShadow {
            anchors.fill: alertWrapper
            source: alertWrapper
        }

        Item {
            id: alertWrapper

            anchors.bottom: updateBtn.top
            anchors.bottomMargin: 32
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width - (Theme.windowMargin * 2)
            height: insecureConnectionAlert.height + 32

            Rectangle {
                anchors.fill: alertWrapper
                color: Theme.white
                radius: 8
            }

            RowLayout {
                id: insecureConnectionAlert

                Layout.minimumHeight: 40
                Layout.fillHeight: true
                Layout.fillWidth: true
                anchors.centerIn: alertWrapper
                spacing: 16

                Image {
                    source: "../resources/connection-info-dark.svg"
                    sourceSize.width: 20
                    sourceSize.height: 20
                    antialiasing: true
                }

                VPNTextBlock {
                    id: alertUpdateRecommendedText

                    font.family: Theme.fontInterFamily
                    font.pixelSize: Theme.fontSizeSmall
                    color: Theme.fontColorDark
                    Layout.maximumWidth: 250
                    Layout.alignment: Qt.AlignVCenter
                    //% "Your connection will not be secure while you update."
                    text: qsTrId("vpn.updates.updateConnectionInsecureWarning")
                }

            }

        }

        VPNButton {
            id: updateBtn

            text: qsTrId("vpn.updates.updateNow")
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: footerLink.top
            anchors.bottomMargin: Theme.hSpacing
            radius: 4
            onClicked: VPN.openLink(VPN.LinkUpdate)
        }

        VPNLinkButton {
            id: footerLink

            //% "Not now"
            readonly property var textNotNow: qsTrId("vpn.updates.notNow")
            //% "Manage account"
            readonly property var textManageAccount: qsTrId("vpn.main.manageAccount")

            labelText: (updatePanel.state === "recommended") ? textNotNow : textManageAccount
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
        }

        VPNSignOut {
            id: signOff

            onClicked: {
                stackview.pop(StackView.Immediate);
                VPNController.logout();
            }
        }

    }

    Behavior on opacity {
        PropertyAnimation {
            duration: 200
        }

    }

    ScrollBar.vertical: ScrollBar {
        policy: windowHeightExceedsContentHeight ? ScrollBar.AlwaysOff : ScrollBar.AlwaysOn
        Accessible.ignored: true
    }

}
