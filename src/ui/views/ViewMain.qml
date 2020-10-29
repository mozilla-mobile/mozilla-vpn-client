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
        flickContentHeight = 444;
        if (alertBox.visible)
            childContentY += alertBox.height + Theme.windowMargin;

        if (mobileHeader.visible)
            childContentY += mobileHeader.height;

    }

    states: [
        State {
            when: window.height > Theme.verticalBreakPoint1

            PropertyChanges {
                target: mobileHeader
                visible: true
            }

            PropertyChanges {
                target: mainContent
                y: {
                    if (alertBox.visible) {
                        mainContent.y = alertBox.height + Theme.windowMargin + mobileHeader.height;
                        alertBox.y = mobileHeader.height + Theme.windowMargin;
                    } else {
                        mainContent.y = mobileHeader.height;
                    }
                }
            }

        },
        State {
            when: window.height < 500

            PropertyChanges {
                target: mobileHeader
                visible: false
            }

            PropertyChanges {
                target: mainContent
                y: {
                    if (alertBox.visible) {
                        mainContent.y = alertBox.height + Theme.windowMargin;
                        alertBox.y = Theme.windowMargin;
                    } else {
                        mainContent.y = 0;
                    }
                }
            }

        }
    ]

    Item {
        id: mobileHeader

        height: 40
        width: parent.width
        anchors.top: parent.top
        anchors.topMargin: Theme.windowMargin / 2
        visible: window.height > Theme.verticalBreakPoint1

        Row {
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 8

            VPNIcon {
                source: "../resources/logo-tray.svg"
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

        function updatePageLayout() {
            let alertHeight = alertBox.height + Theme.windowMargin;
            flickContentHeight -= alertHeight;
            if (!visible && mobileHeader.visible)
                mainContent.y = mobileHeader.height;

            if (!visible && !mobileHeader.visible)
                mainContent.y = 0;

        }

        state: VPN.updateRecommended ? "recommended" : ""
        alertType: "update"
        alertColor: Theme.blueButton
        visible: state === "recommended"
        //% "New version is available."
        alertText: qsTrId("vpn.updates.newVersionAvailable")
        //% "Update now"
        alertLinkText: qsTrId("vpn.updates.updateNow")
        width: parent.width - (Theme.windowMargin * 2)
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
        }

        VPNControllerDevice {
            anchors.top: serverInfo.bottom
            anchors.topMargin: 8
            onClicked: stackview.push("ViewDevices.qml")
        }

        Behavior on y {
            PropertyAnimation {
                duration: 200
                easing.type: Easing.OutCurve
            }

        }

    }
}
