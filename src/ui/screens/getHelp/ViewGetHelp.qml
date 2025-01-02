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

    property string telemetryScreenId: "help"

    _menuTitle: MZI18n.GetHelpLinkText
    _menuOnBackClicked: () => MZNavigator.requestPreviousScreen()

    Component.onCompleted: Glean.impression.helpScreen.record({screen:telemetryScreenId});

    _viewContentData: Column {

        objectName: "getHelpLinks"
        spacing: MZTheme.theme.windowMargin

        Layout.fillWidth: true

        MZExternalLinkListItem {
            objectName: "helpCenter"

            accessibleName: MZI18n.GetHelpHelpCenter
            title: MZI18n.GetHelpHelpCenter
            iconSource: "qrc:/nebula/resources/externalLink.svg"
            backgroundColor: MZTheme.colors.clickableRow
            width: parent.width - MZTheme.theme.windowMargin
            onClicked: MZUrlOpener.openUrlLabel("sumo")
        }

        MZExternalLinkListItem {
            objectName: "inAppSupport"

            accessibleName: MZI18n.InAppSupportWorkflowSupportNavLinkText
            title: MZI18n.InAppSupportWorkflowSupportNavLinkText
            iconSource: MZAssetLookup.getImageSource("Chevron")
            iconMirror: MZLocalizer.isRightToLeft
            backgroundColor: MZTheme.colors.iconButtonLightBackground
            width: parent.width - MZTheme.theme.windowMargin
            onClicked: getHelpStackView.push("qrc:/qt/qml/Mozilla/VPN/screens/getHelp/contactUs/ViewContactUsForm.qml");
        }

        MZExternalLinkListItem {
            objectName: "viewLogs"

            accessibleName: MZI18n.GetHelpViewLogs
            title: MZI18n.GetHelpViewLogs
            iconSource: MZFeatureList.get("shareLogs").isSupported ? "qrc:/nebula/resources/externalLink.svg" : MZAssetLookup.getImageSource("Chevron")
            iconMirror: !MZFeatureList.get("shareLogs").isSupported && MZLocalizer.isRightToLeft
            backgroundColor: MZFeatureList.get("shareLogs").isSupported ?MZTheme.colors.clickableRow : MZTheme.colors.iconButtonLightBackground
            width: parent.width - MZTheme.theme.windowMargin
            onClicked: MZLog.requestViewLogs()
        }

        MZExternalLinkListItem {
            objectName: "resetVpn"

            title: MZI18n.ResetSettingsResetLabel
            iconSource: MZAssetLookup.getImageSource("Chevron")
            iconMirror: MZLocalizer.isRightToLeft
            backgroundColor: MZTheme.colors.iconButtonLightBackground
            visible: MZFeatureList.get("factoryReset").isSupported

            onClicked: getHelpStackView.push("qrc:/qt/qml/Mozilla/VPN/screens/settings/ViewReset.qml");

            accessibleName: title
        }

        MZSettingsItem {
            objectName: "developer"

            width: parent.width - MZTheme.theme.windowMargin
            spacing: MZTheme.theme.listSpacing
            anchors.horizontalCenter: parent.horizontalCenter

            settingTitle: MZI18n.GetHelpDeveloperOptions
            imageLeftSrc: "qrc:/ui/resources/developer.svg"
            imageRightSrc: MZAssetLookup.getImageSource("Chevron")
            imageRightMirror: MZLocalizer.isRightToLeft
            visible: MZSettings.developerUnlock
            onClicked: getHelpStackView.push("qrc:/qt/qml/Mozilla/VPN/screens/getHelp/developerMenu/ViewDeveloperMenu.qml")
        }
    }
}
