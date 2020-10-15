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
    state: VPN.updateRecommended ? "recommended" : "required"

    states: [
        State {
            name: "recommended"
            PropertyChanges {
                target: contentWrapper
                //% "Update recommended"
                logoTitle: qsTrId("updateRecomended")
                //% "Please update the app before you\ncontinue to use the VPN"
                logoSubtitle: qsTrId("updateRecomended.description")
                logoY: 50
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
                    alertBox.visible = true
                    stackview.pop(StackView.Immediate)
                }
                anchors.bottomMargin: 40
            }
        },

        State {
            name: "required"
            PropertyChanges {
                target: contentWrapper
                //% "Update Required"
                logoTitle: qsTrId("updateRequired")
                //% "We detected and fixed a serious bug.\nYou must update your app."
                logoSubtitle: qsTrId("updateRequire.reason")
                logoY: 35
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
                anchors.bottomMargin: 16;
                onClicked: {
                    VPN.openLink(VPN.LinkAccount);
                    // TODO - should anything happen here besides opening
                    // the user account website?
                }
            }
        }
    ]

    contentHeight: (state === "recommended" || !VPN.userAuthenticated) ? 480 : 510
    boundsBehavior: Flickable.StopAtBounds

    opacity: 0
    Component.onCompleted: {
        opacity = 1
    }

    Behavior on opacity {
        PropertyAnimation {
            duration: 200
        }
    }

    VPNPanel {
        id: contentWrapper
        logo: "../resources/update-lock.svg"
        logoY: 35
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
            anchors.top: contentWrapper.top
            anchors.topMargin: contentWrapper.logoY + 36

            Image {
                id: insetImage
                anchors.centerIn: insetCircle
                sourceSize.height: 13
                fillMode: Image.PreserveAspectFit
            }
        }
    }

    VPNDropShadow {
        anchors.fill: alertUpdateRecommendedBox
        source: alertUpdateRecommendedBox
    }


   Item {
        id: alertUpdateRecommendedBox
        anchors.top: parent.top
        anchors.topMargin: contentWrapper.logoY + contentWrapper.childrenRect.height + Theme.vSpacing
        anchors.horizontalCenter: parent.horizontalCenter

        width: parent.width - (Theme.windowMargin * 2)
        height: insecureConnectionAlert.height + 32

        Rectangle {
            anchors.fill: alertUpdateRecommendedBox
            color: Theme.white
            radius: 8
        }

        RowLayout {
            id: insecureConnectionAlert
            Layout.minimumHeight: 40
            Layout.fillHeight: true
            Layout.fillWidth: true
            anchors.centerIn: alertUpdateRecommendedBox
            spacing: 16
            Image {
                id: alertUpdateRecommendedBoxClose
                source: "../resources/connection-info-dark.svg"
                sourceSize.width: 20
                sourceSize.height: 20
                antialiasing: true
            }
            VPNTextBlock {
                id: alertUpdateRecommendedText
                color: Theme.fontColorDark
                Layout.maximumWidth: 250
                Layout.width: 250
                Layout.alignment: Qt.AlignVCenter
                //% "Your connection will not be secure while you update."
                text: qsTrId("updateConnectionInsecureWarning")
            }
        }
    }

    VPNButton {
        id: updateBtn
        width: 282
        text: qsTrId("updateNow")
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: footerLink.top
        anchors.bottomMargin: Theme.vSpacing
        radius: 4
        onClicked: VPN.openLink(VPN.LinkUpdate)
    }

    VPNLinkButton {
        id: footerLink
       
        labelText: (updatePanel.state === "recommended") ? 
            //% "Not now"
            qsTrId("notNow") : 
            //% "Manage account"
            qsTrId("manageAccount")
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
    }

    VPNSignOut {
        id: signOff
        onClicked: {
            stackview.pop(StackView.Immediate)
            VPNController.logout()
        }
    }

     ScrollBar.vertical: ScrollBar {}
}
