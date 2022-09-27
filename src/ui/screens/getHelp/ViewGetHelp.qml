/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1


VPNViewBase {
    //% "Get help"
    _menuTitle: qsTrId("vpn.main.getHelp2")
    _menuOnBackClicked: () => VPNNavigator.requestPreviousScreen()
    _viewContentData: Column {

        objectName: "getHelpLinks"
        spacing: VPNTheme.theme.windowMargin

        Layout.fillWidth: true

        VPNExternalLinkListItem {
            objectName: "helpCenter"

            accessibleName: VPNl18n.GetHelpHelpCenter
            title: VPNl18n.GetHelpHelpCenter
            onClicked: VPNUrlOpener.openLink(VPNUrlOpener.LinkHelpSupport)
            iconSource: "qrc:/nebula/resources/externalLink.svg"
            backgroundColor: VPNTheme.theme.clickableRowBlue
            width: parent.width - VPNTheme.theme.windowMargin
        }

        VPNExternalLinkListItem {
            objectName: "inAppSupport"

            accessibleName: VPNl18n.InAppSupportWorkflowSupportNavLinkText
            title: VPNl18n.InAppSupportWorkflowSupportNavLinkText
            onClicked: getHelpStackView.push("qrc:/ui/screens/getHelp/contactUs/ViewContactUsForm.qml");
            iconSource: "qrc:/nebula/resources/chevron.svg"
            backgroundColor: VPNTheme.theme.iconButtonLightBackground
            width: parent.width - VPNTheme.theme.windowMargin
        }

        VPNExternalLinkListItem {
            objectName: "viewLogs"

            accessibleName: VPNl18n.GetHelpViewLogs
            title: VPNl18n.GetHelpViewLogs
            onClicked: VPN.requestViewLogs()
            iconSource: VPNFeatureList.get("shareLogs").isSupported ? "qrc:/nebula/resources/externalLink.svg" : "qrc:/nebula/resources/chevron.svg"
            backgroundColor: VPNFeatureList.get("shareLogs").isSupported ?VPNTheme.theme.clickableRowBlue : VPNTheme.theme.iconButtonLightBackground
            width: parent.width - VPNTheme.theme.windowMargin
        }

        VPNExternalLinkListItem {
            objectName: "settingsGiveFeedback"

            accessibleName: title
            title: qsTrId("vpn.settings.giveFeedback")
            onClicked: getHelpStackView.push("qrc:/ui/screens/getHelp/giveFeedback/ViewGiveFeedback.qml")
            iconSource: "qrc:/nebula/resources/chevron.svg"
            backgroundColor: VPNTheme.theme.iconButtonLightBackground
            width: parent.width - VPNTheme.theme.windowMargin
            visible: VPN.userState === VPN.UserAuthenticated
        }

        VPNSettingsItem {
            objectName: "developer"

            width: parent.width - VPNTheme.theme.windowMargin
            spacing: VPNTheme.theme.listSpacing
            anchors.horizontalCenter: parent.horizontalCenter

            //% "Developer Options"
            settingTitle: qsTrId("vpn.settings.developer")
            imageLeftSrc: "qrc:/ui/resources/developer.svg"
            imageRightSrc: "qrc:/nebula/resources/chevron.svg"
            visible: VPNSettings.developerUnlock
            onClicked: getHelpStackView.push("qrc:/ui/screens/getHelp/developerMenu/ViewDeveloperMenu.qml")
        }

    }
}
