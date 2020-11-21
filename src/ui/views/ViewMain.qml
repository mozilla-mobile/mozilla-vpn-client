/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.15
import Mozilla.VPN 1.0
import "../components"
import "../themes/themes.js" as Theme

VPNFlickable {
    id: mainView

    flickContentHeight:  {
        let flickHeight = 444;
        if (alertBox.visible) {
            flickHeight += alertBox.height + Theme.windowMargin;
        }

        if (mobileHeader.visible) {
            flickHeight += mobileHeader.height;
        }
        return flickHeight;

    }

    states: [
        State {
            when: window.fullscreenRequired()

            PropertyChanges {
                target: mobileHeader
                visible: true
            }

            PropertyChanges {
                target: mainContent
                y: alertBox.visible ? alertBox.height + Theme.windowMargin + mobileHeader.height : mobileHeader.height
            }
            PropertyChanges {
                target: alertBox
                y: alertBox.visible ? mobileHeader.height + Theme.windowMargin : 0
            }

        },
        State {
            when: !window.fullscreenRequired()

            PropertyChanges {
                target: mobileHeader
                visible: false
            }

            PropertyChanges {
                target: mainContent
                y: alertBox.visible ? alertBox.height + Theme.windowMargin : 0
            }

            PropertyChanges {
                target: alertBox
                y: Theme.windowMargin
            }

        }
    ]

    Item {
        id: mobileHeader

        height: 40
        width: parent.width
        anchors.top: parent.top
        anchors.topMargin: Theme.windowMargin / 2
        visible: window.fullscreenRequired()

        Row {
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 8

            VPNIcon {
                source: VPNStatusIcon.iconUrl
                sourceSize.height: 18
                sourceSize.width: 18
            }

            VPNBoldLabel {
                //% "Mozilla VPN"
                text: qsTrId("MozillaVPN")
                color: "#000000"
                anchors.verticalCenter: parent.verticalCenter
            }

        }

    }

    VPNAlert {
        id: alertBox

        state: VPN.updateRecommended ? "recommended" : ""
        alertType: "update"
        alertColor: Theme.blueButton
        visible: state === "recommended"
        //% "New version is available."
        alertText: qsTrId("vpn.updates.newVersionAvailable")
        //% "Update now"
        alertLinkText: qsTrId("vpn.updates.updateNow")
        width: parent.width - (Theme.windowMargin * 2)

        SequentialAnimation {
            id: closeAlert

            ParallelAnimation {
                PropertyAnimation {
                    target: alertBox
                    property: "opacity"
                    to: 0
                    duration: 100
                }

                PropertyAnimation {
                    target: mainView
                    property: "flickContentHeight"
                    to: mainView.flickContentHeight - alertBox.height - Theme.windowMargin
                    duration: 200
                }

                PropertyAnimation {
                    target: mainContent
                    property: "y"
                    to : {
                        if (mobileHeader.visible) {
                            return mobileHeader.height
                        }
                        return 0
                    }
                    duration: 200
                }
            }
            PropertyAction {
                target: alertBox
                property: "visible"
                value: "false"
            }
        }
    }

    Item {
        id: mainContent

        width: parent.width

        VPNDropShadow {
            anchors.fill: box
            source: box
        }

        VPNControllerView {
            id: box
        }

        VPNControllerServer {
            id: serverInfo

            onClicked: stackview.push("ViewServers.qml")
            y: box.y + box.height + Theme.iconSize
            rowShouldBeDisabled: box.connectionInfoVisible
        }

        VPNControllerDevice {
            anchors.top: serverInfo.bottom
            anchors.topMargin: 8
            onClicked: stackview.push("ViewDevices.qml")
            rowShouldBeDisabled: box.connectionInfoVisible
        }

        Behavior on y {
            PropertyAnimation {
                duration: 200
                easing.type: Easing.OutCurve
            }

        }

    }
}
