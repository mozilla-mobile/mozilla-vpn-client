/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1

MZViewBase {
    id: vpnFlickable
    objectName: "settingsView"
    _accessibleName: MZI18n.NavBarSettingsTab

    _menuTitle: MZI18n.NavBarSettingsTab

    readonly property string telemetryScreenId : "settings"

    _viewContentData: ColumnLayout {
        spacing: MZTheme.theme.windowMargin
        Layout.fillWidth: true

        ColumnLayout {
            Layout.fillWidth: true
            Layout.leftMargin: MZTheme.theme.windowMargin /2
            Layout.rightMargin: MZTheme.theme.windowMargin /2
            Layout.alignment: Qt.AlignHCenter

            MZSettingsItem {
                _loaderVisible: VPNProfileFlow.state === VPNProfileFlow.StateLoading

                property bool subscriptionManagementEnabled: MZFeatureList.get("subscriptionManagement").isSupported
                objectName: "settingsUserProfile"
                settingTitle: MZI18n.SubscriptionManagementSectionTitle
                imageLeftSrc: "qrc:/nebula/resources/avatar-grayscale.svg"
                imageRightSrc: subscriptionManagementEnabled
                    ? "qrc:/nebula/resources/chevron.svg"
                    : "qrc:/nebula/resources/open-in-new.svg"
                imageRightMirror: MZLocalizer.isRightToLeft
                onClicked: {
                    Glean.interaction.accountSelected.record({screen:telemetryScreenId})
                    if (subscriptionManagementEnabled) {
                        VPNProfileFlow.start();
                    } else {
                        Glean.sample.manageAccountClicked.record();
                        MZUrlOpener.openUrlLabel("account");
                    }
                }
            }

            MZSettingsItem {
                objectName: "privacySettings"
                settingTitle: MZI18n.SettingsPrivacySettings
                imageLeftSrc: "qrc:/ui/resources/settings/privacy.svg"
                imageRightSrc: "qrc:/nebula/resources/chevron.svg"
                imageRightMirror: MZLocalizer.isRightToLeft
                onClicked: {
                    Glean.interaction.privacyFeaturesSelected.record({screen:telemetryScreenId})
                    stackview.push("qrc:/ui/screens/settings/privacy/ViewPrivacy.qml")
                }
            }

            MZSettingsItem {
                objectName: "appExclusionSettings"
                settingTitle: MZI18n.SettingsAppExclusionSettings
                imageLeftSrc: "qrc:/ui/resources/settings/apppermissions.svg"
                imageRightSrc: "qrc:/nebula/resources/chevron.svg"
                imageRightMirror: MZLocalizer.isRightToLeft
                onClicked: {
                    Glean.interaction.appExclusionsSelected.record({screen:telemetryScreenId});
                    stackview.push("qrc:/ui/screens/settings/appPermissions/ViewAppPermissions.qml")
                }
                visible: MZFeatureList.get("splitTunnel").isSupported
            }

            MZSettingsItem {
                objectName: "settingsDevice"

                //% "Devices"
                settingTitle: MZI18n.DevicesSectionTitle
                imageLeftSrc: "qrc:/ui/resources/devices.svg"
                imageRightSrc: "qrc:/nebula/resources/chevron.svg"
                imageRightMirror: MZLocalizer.isRightToLeft
                onClicked: {
                    Glean.interaction.myDevicesSelected.record({screen:telemetryScreenId})
                    stackview.push("qrc:/ui/screens/devices/ViewDevices.qml")
                }
            }

            MZSettingsItem {
                id: preferencesSetting
                objectName: "settingsPreferences"
                settingTitle: MZI18n.SettingsPreferencesSettings
                imageLeftSrc: "qrc:/ui/resources/settings/preferences.svg"
                imageRightSrc: "qrc:/nebula/resources/chevron.svg"
                imageRightMirror: MZLocalizer.isRightToLeft
                onClicked: {
                    Glean.interaction.appPreferencesSelected.record({screen:telemetryScreenId})
                    stackview.push("qrc:/ui/screens/settings/ViewPreferences.qml", {
                                                    _startAtBootTitle: Qt.binding(() => MZI18n.SettingsStartAtBootTitle),
                                                    _languageTitle:  Qt.binding(() => qsTrId("vpn.settings.language")),
                                                    _notificationsTitle:  Qt.binding(() => qsTrId("vpn.settings.notifications")),
                                                    _menuTitle: Qt.binding(() => preferencesSetting.settingTitle)
                                                  })
                }
            }

            MZSettingsItem {
                objectName: "settingsGetHelp"
                settingTitle: MZI18n.GetHelpLinkText
                imageLeftSrc: "qrc:/ui/resources/settings/questionMark.svg"
                imageRightSrc: "qrc:/nebula/resources/chevron.svg"
                imageRightMirror: MZLocalizer.isRightToLeft
                onClicked: {
                    Glean.interaction.getHelpSelected.record({screen:telemetryScreenId})
                    MZNavigator.requestScreen(VPN.ScreenGetHelp);
                }
            }

            MZSettingsItem {
                objectName: "settingsAboutUs"
                settingTitle: MZI18n.AboutUsTitle
                imageLeftSrc: "qrc:/ui/resources/settings/aboutUs.svg"
                imageRightSrc: "qrc:/nebula/resources/chevron.svg"
                imageRightMirror: MZLocalizer.isRightToLeft
                onClicked: {
                    Glean.interaction.aboutUsSelected.record({screen:telemetryScreenId})
                    stackview.push("qrc:/ui/screens/settings/ViewAboutUs.qml")
                }
            }

        }
    }

    Connections {
        target: VPNProfileFlow

        function onStateChanged() {
            if (
                VPNProfileFlow.state === VPNProfileFlow.StateReady
                && stackview.currentItem.objectName !== "subscriptionManagmentView"
            ) {
                return stackview.push("qrc:/ui/screens/settings/ViewSubscriptionManagement/ViewSubscriptionManagement.qml");
            }
            // Only push the profile view if it’s not already in the stack
            if (
                VPNProfileFlow.state === VPNProfileFlow.StateAuthenticationNeeded
                && stackview.currentItem.objectName !== "reauthenticationFlow"
            ) {
                return stackview.push("qrc:/ui/screens/settings/ViewSubscriptionManagement/ViewReauthenticationFlow.qml", {
                    _onClose: () => {
                        VPNProfileFlow.reset();
                        stackview.pop(null, StackView.Immediate);
                    }
                });
            }

            // An error occurred during the profile flow. Let’s reset and return
            // to the main settings view.
            const hasError = VPNProfileFlow.state === VPNProfileFlow.StateError;
            if (hasError) {
                if (stackview.currentItem.objectName === "reauthenticationFlow") {
                  stackview.pop(null, StackView.Immediate);
                }
                VPNProfileFlow.reset();
            }
        }
    }
    Component.onCompleted: {
        Glean.impression.settingsScreen.record({screen:telemetryScreenId});
    }
}
