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

    flickContentHeight: col.height + col.anchors.topMargin
    anchors.left: parent.left
    anchors.right: parent.right

    states: [
        State {
            when: window.fullscreenRequired()
            PropertyChanges {
                target: mobileHeader
                visible: true
            }
        },
        State {
            when: !window.fullscreenRequired()
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
                source: "qrc:/ui/resources/logo.svg"
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

            //% "current location - %1"
            //: Accessibility description for current location of the VPN server
            descriptionText: qsTrId("vpn.servers.currentLocation").arg(
                                 VPNCurrentServer.localizedExitCityName)

            disableRowWhen: (VPNController.state !== VPNController.StateOn
                             && VPNController.state !== VPNController.StateOff)
                            || box.connectionInfoScreenVisible
            Layout.topMargin: 12
            contentChildren: [

                VPNServerLabel {
                    id: selectLocationLabel
                    objectName: "serverListButton-label"

                    serversList: [
                        {
                            countryCode: typeof(VPNCurrentServer.entryCountryCode) !== 'undefined' ? VPNCurrentServer.entryCountryCode : "" ,
                            localizedCityName: typeof(VPNCurrentServer.localizedEntryCityName) !== 'undefined' ? VPNCurrentServer.localizedEntryCityName : "",
                            cityName: typeof(VPNCurrentServer.entryCityName) !== "undefined" ? VPNCurrentServer.entryCityName : ""
                        },
                        {
                         countryCode: VPNCurrentServer.exitCountryCode,
                         localizedCityName: VPNCurrentServer.localizedExitCityName,
                         cityName: VPNCurrentServer.exitCityName
                        }
                    ]
                }
            ]
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
                        VPNNavigator.requestScreen(VPNNavigator.ScreenTipsAndTricks);
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
                Glean.sample.tipsAndTricksModalShown.record();
            }

            onClosed: {
                tipsAndTricksIntroPopupLoader.active = false
                MZGleanDeprecated.recordGleanEventWithExtraKeys("tipsAndTricksModalClosed", {"action": closedByPrimaryButton ? "cta" : "dismissed"});
                Glean.sample.tipsAndTricksModalClosed.record({
                    action: closedByPrimaryButton ? "cta" : "dismissed"
                });
            }
        }

        onActiveChanged: if (active) { item.open() }
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
