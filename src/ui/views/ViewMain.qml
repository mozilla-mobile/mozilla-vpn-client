/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import compat 0.1
import telemetry 0.30

VPNFlickable {
    id: vpnFlickable

    objectName: "viewMainFlickable"

    flickContentHeight: col.height + VPNTheme.theme.windowMargin / 2
    anchors.left: parent.left
    anchors.right: parent.right

    states: [
        State {
            when: window.fullscreenRequired()

            PropertyChanges {
                target: mainView
            }
            PropertyChanges {
                target: mobileHeader
                visible: true
            }
        },
        State {
            when: !window.fullscreenRequired()
            PropertyChanges {
                target: mainView
            }
            PropertyChanges {
                target: mobileHeader
                visible: false
            }
        }
    ]

    GridLayout {
        id: col

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: VPNTheme.theme.windowMargin / 2
        anchors.rightMargin: VPNTheme.theme.windowMargin / 2
        flow: GridLayout.TopToBottom
        rowSpacing: VPNTheme.theme.windowMargin
        anchors.top: parent.top
        anchors.topMargin: VPNTheme.theme.windowMargin

        RowLayout {
            id: mobileHeader
            Layout.preferredHeight: VPNTheme.theme.rowHeight
            Layout.alignment: Qt.AlignHCenter
            spacing: 6

            VPNIcon {
                source: VPNStatusIcon.iconUrl
                sourceSize.height: 20
                sourceSize.width: 20
                antialiasing: true
                Layout.alignment: Qt.AlignVCenter
            }

            VPNBoldLabel {
                //% "Mozilla VPN"
                text: qsTrId("MozillaVPN")
                color: "#000000"
                Layout.alignment: Qt.AlignVCenter
            }
        }

        VPNAlerts {
            id: notifications
            updateURL: "qrc:/ui/views/ViewUpdate.qml"
        }

        VPNControllerView {
            id: box
            z: 1
        }

        VPNControllerNav {
            function handleClick() {
                if (disableRowWhen) {
                    return
                }

                stackview.push("ViewServers.qml")
            }

            id: serverInfo

            objectName: "serverListButton"
            btnObjectName: "serverListButton-btn"

            //% "Select location"
            //: Select the Location of the VPN server
            titleText: qsTrId("vpn.servers.selectLocation")

            subtitleText: VPNCurrentServer.localizedCityName

            //% "current location - %1"
            //: Accessibility description for current location of the VPN server
            descriptionText: qsTrId("vpn.servers.currentLocation").arg(
                                 VPNCurrentServer.localizedCityName)

            disableRowWhen: (VPNController.state !== VPNController.StateOn
                             && VPNController.state !== VPNController.StateOff)
                            || box.connectionInfoScreenVisible
            Layout.topMargin: 12
            contentChildren: [

                VPNServerLabel {
                    id: selectLocationLabel
                    serversList: [
                        {
                            countryCode: typeof(VPNCurrentServer.entryCountryCode) !== 'undefined' ? VPNCurrentServer.entryCountryCode : "" ,
                            localizedCityName: typeof(VPNCurrentServer.localizedEntryCity) !== 'undefined' ? VPNCurrentServer.localizedEntryCity : "",
                            cityName: typeof(VPNCurrentServer.entryCityName) !== "undefined" ? VPNCurrentServer.entryCityName : ""
                        },
                        {
                         countryCode: VPNCurrentServer.exitCountryCode,
                         localizedCityName: VPNCurrentServer.localizedCityName,
                         cityName: VPNCurrentServer.exitCityName
                        }
                    ]
                }
            ]
        }

        VPNControllerNav {
            function handleClick() {
                stackview.push("ViewDevices.qml")
            }

            Layout.topMargin: 6

            objectName: "deviceListButton"
            btnObjectName: "deviceListButton-btn"
            //% "My devices"
            titleText: qsTrId("vpn.devices.myDevices")
            disableRowWhen: box.connectionInfoScreenVisible
            contentChildren: [
                VPNIcon {
                    source: "qrc:/nebula/resources/devices.svg"
                    Layout.alignment: Qt.AlignLeft | Qt.AlignCenter
                },

                VPNLightLabel {
                    id: serverLocation
                    objectName: "deviceListButtonLabel"
                    Accessible.ignored: true
                    Layout.alignment: Qt.AlignLeft
                    elide: Text.ElideRight
                    //% "%1 of %2"
                    //: Example: You have "x of y" devices in your account, where y is the limit of allowed devices.
                    text: qsTrId("vpn.devices.activeVsMaxDeviceCount").arg(
                              VPNDeviceModel.activeDevices
                              + (VPN.state !== VPN.StateDeviceLimit ? 0 : 1)).arg(
                              VPNUser.maxDevices)
                }
            ]
        }

        VPNVerticalSpacer {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
        }
    }

    //Tips and tricks popup
    Loader {
        id: tipsAndTricksIntroPopupLoader

        objectName: "tipsAndTricksIntroPopupLoader"
        active: false
        sourceComponent: VPNSimplePopup {
            id: tipsAndTricksIntroPopup

            //Keeps track of how the popup was closed to determine whether or not to fire "dismissed" telemetry
            property bool closedByPrimaryButton: false

            anchors.centerIn: Overlay.overlay
            closeButtonObjectName: "tipsAndTricksIntroPopupCloseButton"
            imageSrc: "qrc:/ui/resources/logo-sparkles.svg"
            imageSize: Qt.size(116, 80)
            title: VPNl18n.TipsAndTricksSettingsEntryLabel
            description: VPNl18n.TipsAndTricksIntroModalDescription
            buttons: [
                VPNButton {
                    id: tipAndTricksIntroButton
                    objectName: "tipsAndTricksIntroPopupDiscoverNowButton"
                    text: VPNl18n.GlobalDiscoverNow
                    onClicked: {
                        tipAndTricksIntroButton.enabled = false
                        closedByPrimaryButton = true
                        tipsAndTricksIntroPopup.close()
                        mainStackView.push(tipsAndTricksDeepLinkView)
                    }
                },
                VPNLinkButton {
                    objectName: "tipsAndTricksIntroPopupGoBackButton"
                    labelText: VPNl18n.GlobalGoBack
                    onClicked: tipsAndTricksIntroPopup.close()
                }
            ]

            onOpened: {
                VPNSettings.tipsAndTricksIntroShown = true
                Sample.tipsAndTricksModalShown.record();
            }

            onClosed: {
                tipsAndTricksIntroPopupLoader.active = false
                VPN.recordGleanEventWithExtraKeys("tipsAndTricksModalClosed", {"action": closedByPrimaryButton ? "cta" : "dismissed"});
            }
        }

        onActiveChanged: if (active) { item.open() }

        Component {
            id: tipsAndTricksDeepLinkView

            ColumnLayout {

                spacing: 0

                VPNMenu {
                    id: menu
                    objectName: "tipsAndTricksCloseButton"

                    Layout.fillWidth: true

                    _iconButtonSource:"qrc:/nebula/resources/close-dark.svg"
                    title: VPNl18n.TipsAndTricksSettingsEntryLabel
                    _menuOnBackClicked: () => { mainStackView.pop() }
                }

                Loader {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    source: "qrc:/ui/settings/ViewTipsAndTricks/ViewTipsAndTricks.qml"
                }
            }
        }
    }

    function maybeActivateTipsAndTricksIntro() {
        if (!VPNSettings.tipsAndTricksIntroShown &&
            VPNAddonManager.loadCompleted &&
            !!VPNAddonManager.pick(addon => addon.type === "tutorial" || addon.type === "guide")) {
            tipsAndTricksIntroPopupLoader.active = true
        }
    }

    Component.onCompleted: () => maybeActivateTipsAndTricksIntro();

    Connections {
        target: VPNAddonManager
        function onLoadCompletedChanged() { maybeActivateTipsAndTricksIntro(); }
    }
}
