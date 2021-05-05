/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0
import "../components"
import "../themes/themes.js" as Theme
import "/glean/load.js" as Glean

VPNFlickable {
    id: vpnFlickable
    objectName: "settingsView"

    width: window.width
    flickContentHeight: settingsList.y + settingsList.height + signOutLink.height + signOutLink.anchors.bottomMargin
    hideScollBarOnStackTransition: true

    VPNIconButton {
        id: iconButton
        objectName: "settingsCloseButton"

        onClicked: stackview.pop(StackView.Immediate)
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.topMargin: Theme.windowMargin / 2
        anchors.leftMargin: Theme.windowMargin / 2
        accessibleName: qsTrId("vpn.connectionInfo.close")

        Image {
            id: backImage

            source: "../resources/close-dark.svg"
            sourceSize.width: Theme.iconSize
            fillMode: Image.PreserveAspectFit
            anchors.centerIn: iconButton
        }

    }

    VPNPanel {
        id: vpnPanel
        logoSize: 80
        logo:  VPNUser.avatar
        //% "VPN User"
        readonly property var textVpnUser: qsTrId("vpn.settings.user")
        logoTitle: VPNUser.displayName ? VPNUser.displayName : textVpnUser
        logoSubtitle: VPNUser.email
        anchors.top: parent.top
        anchors.topMargin: (Math.max(window.safeContentHeight * .08, Theme.windowMargin * 2))
        maskImage: true
        isSettingsView: true
    }

    VPNButton {
        id: manageAccountButton
        objectName: "manageAccountButton"

        text: qsTrId("vpn.main.manageAccount")
        anchors.top: vpnPanel.bottom
        anchors.topMargin: Theme.vSpacing
        anchors.horizontalCenter: parent.horizontalCenter
        onClicked: {
            Glean.sample.manageAccountClicked.record();
            VPN.openLink(VPN.LinkAccount)
        }
    }

    VPNCheckBoxRow {
        id: startAtBootCheckBox
        objectName: "settingStartAtBoot"

        //% "Launch VPN app on Startup"
        labelText: qsTrId("vpn.settings.runOnBoot")
        subLabelText: ""
        isChecked: VPNSettings.startAtBoot
        isEnabled: true
        showDivider: true
        anchors.top: manageAccountButton.bottom
        anchors.topMargin: Theme.hSpacing * 1.5
        anchors.rightMargin: Theme.hSpacing
        width: vpnFlickable.width - Theme.hSpacing
        onClicked: VPNSettings.startAtBoot = !VPNSettings.startAtBoot
        visible: VPNFeatureList.startOnBootSupported
    }

    VPNCheckBoxRow {
        id: gleanEnabledCheckBox
        objectName: "settingGleanEnabled"

        //% "Enable telemetry"
        labelText: qsTrId("vpn.settings.glean")
        subLabelText: ""
        isChecked: VPNSettings.gleanEnabled
        isEnabled: true
        showDivider: true
        anchors.top: VPNFeatureList.startOnBootSupported ? startAtBootCheckBox.bottom : manageAccountButton.bottom
        anchors.topMargin: Theme.hSpacing * 1.5
        anchors.rightMargin: Theme.hSpacing
        width: vpnFlickable.width - Theme.hSpacing
        onClicked: {
            VPNSettings.gleanEnabled = !VPNSettings.gleanEnabled
            Glean.setUploadEnabled(VPNSettings.gleanEnabled);
        }
        visible: VPNFeatureList.gleanSupported
    }

    Component {
        id: getHelpComponent

        VPNGetHelp {
            isSettingsView: true
        }

    }

    Component {
        id: aboutUsComponent

        VPNAboutUs {
            isSettingsView: true
        }
    }


    ColumnLayout {
        id: settingsList

        spacing: Theme.listSpacing
        y: Theme.vSpacing + (VPNFeatureList.gleanSupported ? gleanEnabledCheckBox.y + gleanEnabledCheckBox.height :
                              (VPNFeatureList.startOnBootSupported ? startAtBootCheckBox.y + startAtBootCheckBox.height :
                                manageAccountButton.y + manageAccountButton.height))
        width: parent.width - Theme.windowMargin
        anchors.horizontalCenter: parent.horizontalCenter

        VPNSettingsItem {
            objectName: "settingsNetworking"

            settingTitle: qsTrId("vpn.settings.networking")
            imageLeftSrc: "../resources/settings/networkSettings.svg"
            imageRightSrc: "../resources/chevron.svg"
            onClicked: settingsStackView.push("../settings/ViewNetworkSettings.qml")
        }
        VPNSettingsItem {
            objectName: "settingsNotifications"

            settingTitle: qsTrId("vpn.settings.notifications")
            imageLeftSrc: "../resources/settings/notifications.svg"
            imageRightSrc: "../resources/chevron.svg"
            onClicked: settingsStackView.push("../settings/ViewNotifications.qml")
	    visible: VPNFeatureList.captivePortalNotificationSupported || VPNFeatureList.unsecuredNetworkNotificationSupported
        }
        VPNSettingsItem {
            objectName: "settingsLanguages"

            settingTitle: qsTrId("vpn.settings.language")
            imageLeftSrc: "../resources/settings/language.svg"
            imageRightSrc: "../resources/chevron.svg"
            onClicked: settingsStackView.push("../settings/ViewLanguage.qml")
            visible: VPNLocalizer.hasLanguages
        }
        VPNSettingsItem {
            //% "App Permissions"
            settingTitle: qsTrId("vpn.settings.appPermissions")
            imageLeftSrc: "../resources/settings/apps.svg"
            imageRightSrc: "../resources/chevron.svg"
            visible: VPNFeatureList.protectSelectedAppsSupported
            onClicked: settingsStackView.push("../settings/ViewAppPermissions.qml")
        }
        VPNSettingsItem {
            objectName: "settingsAboutUs"

            settingTitle: qsTrId("vpn.settings.aboutUs")
            imageLeftSrc: "../resources/settings/aboutUs.svg"
            imageRightSrc: "../resources/chevron.svg"
            onClicked: settingsStackView.push(aboutUsComponent)
        }
        VPNSettingsItem {
            objectName: "settingsGiveFeedback"

            //% "Give feedback"
            settingTitle: qsTrId("vpn.settings.giveFeedback")
            imageLeftSrc: "../resources/settings/feedback.svg"
            imageRightSrc: "../resources/externalLink.svg"
            onClicked: VPN.openLink(VPN.LinkFeedback)
        }
        VPNSettingsItem {
            objectName: "settingsGetHelp"

            settingTitle: qsTrId("vpn.main.getHelp")
            imageLeftSrc: "../resources/settings/getHelp.svg"
            imageRightSrc: "../resources/chevron.svg"
            onClicked: settingsStackView.push(getHelpComponent)
        }

        Rectangle {
            Layout.preferredHeight: fullscreenRequired? Theme.rowHeight * 1.5 : Theme.rowHeight
            Layout.fillWidth: true
            color: "transparent"
        }
    }

    VPNSignOut {
        id: signOutLink

        objectName: "settingsLogout"
        onClicked: VPNController.logout()
    }

}
