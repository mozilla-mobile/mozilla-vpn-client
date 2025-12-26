/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1
import "qrc:/nebula/utils/MZAssetLookup.js" as MZAssetLookup

MZViewBase {
    id: vpnFlickable
    property string _startAtBootTitle: ""
    property string _notificationsTitle: ""
    property string _languageTitle: ""

    property bool vpnIsOff: VPNController.state === VPNController.StateOff
    property bool isIOS: Qt.platform.os === "ios"

    objectName: "settingsPreferencesView"

    _viewContentData: ColumnLayout {
        Layout.preferredWidth: parent.width
        Layout.alignment: Qt.AlignHCenter

        spacing: MZTheme.theme.windowMargin

        MZLinkRow {
            objectName: "androidStartAtBootLink"

            title: _startAtBootTitle
            subLabelText: MZI18n.SettingsStartAtBootSubtitle
            visible: Qt.platform.os === "android"
            onClicked: MZUrlOpener.openUrlLabel("sumoAlwaysOnAndroid")
        }

        MZToggleRow {
            objectName: "startAtBootToogle"

            Layout.fillWidth: true
            Layout.rightMargin: MZTheme.theme.windowMargin
            Layout.leftMargin: MZTheme.theme.windowMargin

            labelText: _startAtBootTitle
            subLabelText: MZI18n.SettingsStartAtBootSubtitle
            checked: MZSettings.startAtBoot
            visible: MZFeatureList.get("startOnBoot").isSupported
            dividerTopMargin: MZTheme.theme.toggleRowDividerSpacing
            onClicked: {
                MZSettings.startAtBoot = !MZSettings.startAtBoot
            }
        }

        MZToggleRow {
            objectName: "dataCollectionToggle"

            Layout.fillWidth: true
            Layout.rightMargin: MZTheme.theme.windowMargin
            Layout.leftMargin: MZTheme.theme.windowMargin
            showDivider: isIOS

            labelText: MZI18n.TelemetryPolicyViewShareTechnicalData
            subLabelText: MZI18n.SettingsShareTechnicalDataDescription
            checked: MZSettings.gleanEnabled
            dividerTopMargin: MZTheme.theme.toggleRowDividerSpacing
            onClicked: MZSettings.gleanEnabled = !MZSettings.gleanEnabled
        }

        MZToggleRow {
            id: localNetwork
            objectName: "settingLocalNetworkAccess"
            visible: isIOS

            Layout.fillWidth: true
            Layout.rightMargin: MZTheme.theme.windowMargin
            Layout.leftMargin: MZTheme.theme.windowMargin
            showDivider: false

            labelText: MZI18n.BypassVPNLabelTitle
            subLabelText: MZI18n.BypassVPNSubLabel
            linkButtonText: MZI18n.BypassVPNLearnMoreSentence
            checked: MZSettings.localNetworkAccess
            toggleEnabled: isIOS && vpnIsOff
            dividerTopMargin: MZTheme.theme.toggleRowDividerSpacing
            onClicked: {
                if (VPNController.StateOff) {
                    MZSettings.localNetworkAccess = !MZSettings.localNetworkAccess
                }
            }
            onClickedWhileDisabled: MZErrorHandler.requestAlert(MZErrorHandler.VPNMustBeOffWarning)
            onButtonClicked: { MZUrlOpener.openUrl("https://support.mozilla.org/kb/enable-or-disable-local-network-access-ios") }
        }

        ColumnLayout {
            spacing: MZTheme.theme.windowMargin / 2
            Layout.alignment: Qt.AlignHCenter
            Layout.leftMargin: MZTheme.theme.windowMargin / 2
            Layout.rightMargin: MZTheme.theme.windowMargin / 2

            MZSettingsItem {
                objectName: "settingsNotifications"

                settingTitle: _notificationsTitle
                imageLeftSrc: MZAssetLookup.getImageSource("IconBell")
                imageRightSrc: MZAssetLookup.getImageSource("Chevron")
                imageRightMirror: MZLocalizer.isRightToLeft
                onClicked: {
                    if(Qt.platform.os === "android"){
                        VPNAndroidUtils.openNotificationSettings();
                        return;
                    }
                    stackview.push("qrc:/qt/qml/Mozilla/VPN/screens/settings/ViewNotifications.qml")
                }
                visible: MZFeatureList.get("captivePortal").isSupported || MZFeatureList.get("unsecuredNetworkNotification").isSupported || MZFeatureList.get("notificationControl").isSupported
            }

            MZSettingsItem {
                objectName: "settingsLanguages"

                settingTitle: _languageTitle
                imageLeftSrc: MZAssetLookup.getImageSource("IconLanguages")
                imageRightSrc: MZAssetLookup.getImageSource("Chevron")
                imageRightMirror: MZLocalizer.isRightToLeft
                onClicked: {
                    stackview.push("qrc:/qt/qml/Mozilla/VPN/screens/settings/ViewLanguage.qml")
                }
                visible: MZLocalizer.hasLanguages
            }

            MZSettingsItem {
                objectName: "dnsSettings"

                settingTitle: MZI18n.SettingsDnsSettings
                imageLeftSrc: MZAssetLookup.getImageSource("IconGlobe")
                imageRightSrc: MZAssetLookup.getImageSource("Chevron")
                imageRightMirror: MZLocalizer.isRightToLeft
                onClicked: {
                    stackview.push("qrc:/qt/qml/Mozilla/VPN/screens/settings/ViewDNSSettings.qml")
                }
            }

            MZSettingsItem {
                objectName: "appearance"

                settingTitle: MZI18n.SettingsAppearance
                imageLeftSrc: MZAssetLookup.getImageSource("IconPaintbrush")
                imageRightSrc: MZAssetLookup.getImageSource("Chevron")
                imageRightMirror: MZLocalizer.isRightToLeft
                onClicked: {
                    stackview.push("qrc:/qt/qml/Mozilla/VPN/screens/settings/ViewAppearance.qml")
                }
            }
        }
    }
}
