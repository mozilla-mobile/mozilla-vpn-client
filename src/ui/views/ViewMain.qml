/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.14
import Mozilla.VPN 1.0
import "../components"
import "../themes/themes.js" as Theme

VPNFlickable {
    id: mainView

    flickContentHeight:  {
        let flickHeight = Theme.desktopAppHeight - 1
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

        height: Theme.rowHeight * 1.5
        width: parent.width
        anchors.top: parent.top
        anchors.topMargin: Theme.windowMargin / 2
        visible: window.fullscreenRequired()

        Row {
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 6

            VPNIcon {
                source: VPNStatusIcon.iconUrl
                sourceSize.height: 20
                sourceSize.width: 20
                antialiasing: true
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
        onVisibleChanged: if (visible) showAlert.start();
        //% "New version is available."
        alertText: qsTrId("vpn.updates.newVersionAvailable")
        //% "Update now"
        alertLinkText: qsTrId("vpn.updates.updateNow")
        width: parent.width - (Theme.windowMargin * 2)

        PropertyAnimation {
            id: showAlert

            target: alertBox
            property: "opacity"
            from: 0
            to: 1
            duration: 200
        }

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

        VPNControllerNav {
            function handleClick() {
                stackview.push("ViewServers.qml")
            }

            id: serverInfo
            objectName: "serverListButton"

            //% "Select location"
            //: Select the Location of the VPN server
            titleText: qsTrId("vpn.servers.selectLocation")
            //% "current location - %1"
            //: Accessibility description for current location of the VPN server
            descriptionText: qsTrId("vpn.servers.currentLocation").arg(VPNLocalizer.translateServerCity(VPNCurrentServer.countryCode, VPNCurrentServer.city))

            subtitleText: VPNLocalizer.translateServerCity(VPNCurrentServer.countryCode, VPNCurrentServer.city)
            imgSource:  "../resources/flags/" + VPNCurrentServer.countryCode.toUpperCase() + ".png"
            anchors.top: box.bottom
            anchors.topMargin: 30

            disableRowWhen: (VPNController.state !== VPNController.StateOn && VPNController.state !== VPNController.StateOff) || box.connectionInfoVisible
        }

        VPNControllerNav {
            function handleClick() {
                stackview.push("ViewDevices.qml")
            }

            objectName: "deviceListButton"
            anchors.top: serverInfo.bottom
            anchors.topMargin: 22
            //% "%1 of %2"
            //: Example: You have "x of y" devices in your account, where y is the limit of allowed devices.
            subtitleText: qsTrId("vpn.devices.activeVsMaxDeviceCount").arg(VPNDeviceModel.activeDevices + (VPN.state !== VPN.StateDeviceLimit ? 0 : 1)).arg(VPNUser.maxDevices)
            imgSource: "../resources/devices.svg"
            imgIsVector: true
            imgSize: 24
            //% "My devices"
            titleText: qsTrId("vpn.devices.myDevices")
            disableRowWhen: box.connectionInfoVisible
        }

        Behavior on y {
            PropertyAnimation {
                duration: 200
                easing.type: Easing.OutCurve
            }

        }

    }
}
