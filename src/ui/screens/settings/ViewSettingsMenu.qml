/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import "qrc:/nebula/utils/MZAssetLookup.js" as MZAssetLookup

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
                imageLeftSrc: MZAssetLookup.getImageSource("IconAvatar")
                imageRightSrc: subscriptionManagementEnabled
                    ? MZAssetLookup.getImageSource("Chevron")
                    : MZAssetLookup.getImageSource("ExternalLinkGrayscale")
                imageRightMirror: MZLocalizer.isRightToLeft
                onClicked: {
                    Glean.interaction.accountSelected.record({screen:telemetryScreenId})
                    if (subscriptionManagementEnabled) {
                        VPNProfileFlow.start();
                    } else {
                        MZUrlOpener.openUrlLabel("account");
                    }
                }
            }

            MZSettingsItem {
                objectName: "privacySettings"
                settingTitle: MZI18n.SettingsPrivacySettings
                imageLeftSrc: MZAssetLookup.getImageSource("IconPrivacyMask")
                imageRightSrc: MZAssetLookup.getImageSource("Chevron")
                imageRightMirror: MZLocalizer.isRightToLeft
                onClicked: {
                    Glean.interaction.privacyFeaturesSelected.record({screen:telemetryScreenId})
                    stackview.push("qrc:/qt/qml/Mozilla/VPN/screens/settings/privacy/ViewPrivacy.qml")
                }
            }

            MZSettingsItem {
                objectName: "appExclusionSettings"
                settingTitle: MZI18n.SettingsAppExclusionTitle
                imageLeftSrc: MZAssetLookup.getImageSource("IconPermissions")
                imageRightSrc: MZAssetLookup.getImageSource("Chevron")
                imageRightMirror: MZLocalizer.isRightToLeft
                onClicked: {
                    Glean.interaction.appExclusionsSelected.record({screen:telemetryScreenId});
                    stackview.push("qrc:/qt/qml/Mozilla/VPN/screens/settings/appPermissions/ViewAppPermissions.qml")
                }
                visible: MZFeatureList.get("splitTunnel").isSupported
            }

            MZSettingsItem {
                objectName: "settingsDevice"

                //% "Devices"
                settingTitle: MZI18n.DevicesSectionTitle
                imageLeftSrc: MZAssetLookup.getImageSource("IconDevices")
                imageRightSrc: MZAssetLookup.getImageSource("Chevron")
                imageRightMirror: MZLocalizer.isRightToLeft
                onClicked: {
                    Glean.interaction.myDevicesSelected.record({screen:telemetryScreenId})
                    stackview.push("qrc:/qt/qml/Mozilla/VPN/screens/devices/ViewDevices.qml")
                }
            }

            MZSettingsItem {
                id: preferencesSetting
                objectName: "settingsPreferences"
                settingTitle: MZI18n.SettingsPreferencesSettings
                imageLeftSrc: MZAssetLookup.getImageSource("IconWrenchDarker")
                imageRightSrc: MZAssetLookup.getImageSource("Chevron")
                imageRightMirror: MZLocalizer.isRightToLeft
                onClicked: {
                    Glean.interaction.appPreferencesSelected.record({screen:telemetryScreenId})
                    stackview.push("qrc:/qt/qml/Mozilla/VPN/screens/settings/ViewPreferences.qml", {
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
                imageLeftSrc: MZAssetLookup.getImageSource("MenuIconQuestion")
                imageRightSrc: MZAssetLookup.getImageSource("Chevron")
                imageRightMirror: MZLocalizer.isRightToLeft
                onClicked: {
                    Glean.interaction.getHelpSelected.record({screen:telemetryScreenId})
                    MZNavigator.requestScreen(VPN.ScreenGetHelp);
                }
            }

            MZSettingsItem {
                objectName: "settingsAboutUs"
                settingTitle: MZI18n.AboutUsTitle
                imageLeftSrc: MZAssetLookup.getImageSource("InfoIconDarker")
                imageRightSrc: MZAssetLookup.getImageSource("Chevron")
                imageRightMirror: MZLocalizer.isRightToLeft
                onClicked: {
                    Glean.interaction.aboutUsSelected.record({screen:telemetryScreenId})
                    stackview.push("qrc:/qt/qml/Mozilla/VPN/screens/settings/ViewAboutUs.qml")
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
                reauthPopup.close();
                return stackview.push("qrc:/qt/qml/Mozilla/VPN/screens/settings/ViewSubscriptionManagement/ViewSubscriptionManagement.qml");
            }
            // Only push the profile view if it’s not already in the stack
            if (VPNProfileFlow.state === VPNProfileFlow.StateAuthenticationNeeded) {
              if (!MZFeatureList.get("inAppAuthentication").isSupported) {
                reauthPopup.open();
              } else if (stackview.currentItem.objectName !== "reauthenticationFlow") {
                reauthPopup.close();
                return stackview.push("qrc:/qt/qml/Mozilla/VPN/screens/settings/ViewSubscriptionManagement/ViewReauthenticationFlow.qml", {
                    _onClose: () => {
                        VPNProfileFlow.reset();
                        stackview.pop(null, StackView.Immediate);
                    }
                });
              }
            }

            // An error occurred during the profile flow. Let’s reset and return
            // to the main settings view.
            const hasError = VPNProfileFlow.state === VPNProfileFlow.StateError;
            if (hasError) {
                reauthPopup.close();
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

    MZSimplePopup {
        id: reauthPopup

        anchors.centerIn: Overlay.overlay
        imageSrc: MZAssetLookup.getImageSource("RefreshArrowsWithWarning")
        imageSize: Qt.size(80, 80)
        title: MZI18n.SettingsReauthTitle
        description: MZI18n.SettingsReauthDescription
        buttons: [
            MZButton {
                id: reauthButton
                objectName: "reauthButton"
                text: MZI18n.InAppAuthContinueToSignIn
                Layout.fillWidth: true
                onClicked: {
                  VPNProfileFlow.reauthenticateViaWeb();
                  loader.state = "active";
                  reauthButton.enabled = false;
                }

                Rectangle {
                  width: MZTheme.theme.rowHeight
                  height: MZTheme.theme.rowHeight
                  anchors.right: parent.right
                  color: MZTheme.colors.transparent

                  MZButtonLoader {
                    id: loader
                    color: MZTheme.colors.transparent
                    iconUrl: MZAssetLookup.getImageSource("Spinner")
                  }
               }
            }
        ]

        onOpened: {
          loader.state = "inactive";
          reauthButton.enabled = true;
        }

        onClosed: {
          VPNProfileFlow.reset();
        }
    }
}
