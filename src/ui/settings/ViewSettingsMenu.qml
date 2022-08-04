/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1


VPNFlickable {
    id: vpnFlickable
    objectName: "settingsView"
    flickContentHeight: settingsList.implicitHeight

    Component {
        id: aboutUsComponent

        VPNAboutUs {
            licenseURL: "qrc:/ui/views/ViewLicenses.qml"
        }
    }

    ColumnLayout {
        id: settingsList

        spacing: VPNTheme.theme.windowMargin
        width: parent.width
        height: Math.max(vpnFlickable.height, settingsList.implicitHeight)

        anchors {
            top: parent.top
            horizontalCenter: parent.horizontalCenter
        }

        ColumnLayout {
            spacing: 0
            Layout.fillWidth: true

            VPNVerticalSpacer {
                Layout.preferredHeight: VPNTheme.theme.windowMargin * 1
            }

            VPNUserProfile {
                property bool subscriptionManagementEnabled: VPNFeatureList.get("subscriptionManagement").isSupported
                objectName: "settingsUserProfile"

                enabled: VPNProfileFlow.state === VPNProfileFlow.StateInitial

                _iconSource: subscriptionManagementEnabled
                    ? "qrc:/nebula/resources/chevron.svg"
                    : "qrc:/nebula/resources/open-in-new.svg"
                _buttonOnClicked: () => {
                    if (subscriptionManagementEnabled) {
                        VPNProfileFlow.start();
                    } else {
                        VPN.recordGleanEvent("manageAccountClicked")
                        VPN.openLink(VPN.LinkAccount);
                    }
                }
                _loaderVisible: VPNProfileFlow.state === VPNProfileFlow.StateLoading

            }

            VPNVerticalSpacer {
                Layout.preferredHeight: VPNTheme.theme.windowMargin * 1
            }

            Rectangle {
                id: divider

                Layout.preferredHeight: 1
                Layout.fillWidth: true
                Layout.leftMargin: VPNTheme.theme.windowMargin / 2
                Layout.rightMargin: VPNTheme.theme.windowMargin / 2
                color: VPNTheme.colors.grey10
            }
        }


        // TODO: Move to subscription management
        ColumnLayout {
//            Layout.fillWidth: true
            Layout.preferredWidth: parent.width - VPNTheme.theme.windowMargin
            Layout.maximumWidth: parent.width - VPNTheme.theme.windowMargin
            Layout.alignment: Qt.AlignHCenter

            VPNSettingsItem {
                objectName: "settingsTipsAndTricks"
                settingTitle: VPNl18n.TipsAndTricksSettingsEntryLabel
                imageLeftSrc: "qrc:/nebula/resources/sparkles.svg"
                imageRightSrc: "qrc:/nebula/resources/chevron.svg"
                onClicked: settingsStack.push("qrc:/ui/settings/ViewTipsAndTricks/ViewTipsAndTricks.qml")
            }

            VPNSettingsItem {
                objectName: "settingsNetworking"
                settingTitle: qsTrId("vpn.settings.networking")
                imageLeftSrc: "qrc:/ui/resources/settings/networkSettings.svg"
                imageRightSrc: "qrc:/nebula/resources/chevron.svg"
                onClicked: settingsStack.push("qrc:/ui/settings/ViewNetworkSettings.qml", {
                                                      //% "App permissions"
                                                      _appPermissionsTitle: Qt.binding(() => qsTrId("vpn.settings.appPermissions2"))
                                                  })
            }

            VPNSettingsItem {
                id: preferencesSetting
                objectName: "settingsPreferences"
                settingTitle: VPNl18n.SettingsSystemPreferences
                imageLeftSrc: "qrc:/ui/resources/settings/preferences.svg"
                imageRightSrc: "qrc:/nebula/resources/chevron.svg"
                onClicked: settingsStack.push("qrc:/ui/settings/ViewPrivacySecurity.qml", {
                                                    _startAtBootTitle: Qt.binding(() => VPNl18n.SettingsStartAtBootTitle),
                                                    _languageTitle:  Qt.binding(() => qsTrId("vpn.settings.language")),
                                                    _notificationsTitle:  Qt.binding(() => qsTrId("vpn.settings.notifications")),
                                                    _menuTitle: Qt.binding(() => preferencesSetting.settingTitle)
                                                  })
            }

            VPNSettingsItem {
                objectName: "deviceListButton"
                settingTitle: qsTrId("vpn.devices.myDevices")
                imageLeftSrc: "qrc:/nebula/resources/devices.svg"
                imageRightSrc: "qrc:/nebula/resources/chevron.svg"
                onClicked: settingsStack.push("qrc:/ui/views/ViewDevices.qml")
            }

            VPNSettingsItem {
                //% "Give feedback"
                property string giveFeedbackTitle: qsTrId("vpn.settings.giveFeedback")
                objectName: "settingsGetHelp"
                settingTitle: qsTrId("vpn.main.getHelp2")
                imageLeftSrc: "qrc:/ui/resources/settings/questionMark.svg"
                imageRightSrc: "qrc:/nebula/resources/chevron.svg"
                onClicked: {
                    VPN.recordGleanEvent("getHelpClickedViewSettings");
                    settingsStack.push("qrc:/ui/views/ViewGetHelp.qml")
                }
            }

            VPNSettingsItem {
                objectName: "settingsAboutUs"
                settingTitle: qsTrId("vpn.settings.aboutUs")
                imageLeftSrc: "qrc:/ui/resources/settings/aboutUs.svg"
                imageRightSrc: "qrc:/nebula/resources/chevron.svg"
                onClicked: settingsStack.push(aboutUsComponent)
            }

            VPNSignOut {
                id: signOutLink

                objectName: "settingsLogout"
                anchors {
                    horizontalCenter: undefined
                    bottom: undefined
                    bottomMargin: undefined
                }
                Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
                Layout.topMargin: VPNTheme.theme.vSpacing
            }

            VPNVerticalSpacer {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
    }

    Connections {
        target: VPNProfileFlow

        function onStateChanged() {
            if (
                VPNProfileFlow.state === VPNProfileFlow.StateReady
                && settingsStack.currentItem.objectName !== "subscriptionManagmentView"
            ) {
                return settingsStack.push("qrc:/ui/settings/ViewSubscriptionManagement/ViewSubscriptionManagement.qml");
            }

            // Only push the profile view if it’s not already in the stack
            if (
                VPNProfileFlow.state === VPNProfileFlow.StateAuthenticationNeeded
                && mainStackView.currentItem.objectName !== "reauthenticationFlow"
            ) {
                return mainStackView.push("qrc:/ui/authenticationInApp/ViewReauthenticationFlow.qml", {
                    _targetViewCondition: Qt.binding(() => VPNProfileFlow.state === VPNProfileFlow.StateReady),
                    _onClose: () => {
                        VPNProfileFlow.reset();
                        mainStackView.pop();
                    }
                });
            }

            // An error occurred during the profile flow. Let’s reset and return
            // to the main settings view.
            const hasError = VPNProfileFlow.state === VPNProfileFlow.StateError;
            if (hasError) {
                if (mainStackView.currentItem.objectName === "reauthenticationFlow") {
                    mainStackView.pop();
                }
                VPNProfileFlow.reset();
            }
        }
    }
}
