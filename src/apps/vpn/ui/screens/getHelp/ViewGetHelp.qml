/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1


VPNViewBase {
    id: vpnFlickable
    //% "Get help"
    _menuTitle: qsTrId("vpn.main.getHelp2")
    _menuOnBackClicked: () => VPNNavigator.requestPreviousScreen()
    _viewContentData: Column {

        objectName: "getHelpLinks"
        spacing: VPNTheme.theme.windowMargin

        Layout.fillWidth: true

        VPNExternalLinkListItem {
            objectName: "helpCenter"

            accessibleName: VPNI18n.GetHelpHelpCenter
            title: VPNI18n.GetHelpHelpCenter
            iconSource: "qrc:/nebula/resources/externalLink.svg"
            backgroundColor: VPNTheme.theme.clickableRowBlue
            width: parent.width - VPNTheme.theme.windowMargin
            onClicked: {
                VPNGleanDeprecated.recordGleanEvent("helpMenuHelpCenterOpened");
                Glean.sample.helpMenuHelpCenterOpened.record();
                VPNUrlOpener.openUrlLabel("sumo")
            }
        }

        VPNExternalLinkListItem {
            objectName: "inAppSupport"

            accessibleName: VPNI18n.InAppSupportWorkflowSupportNavLinkText
            title: VPNI18n.InAppSupportWorkflowSupportNavLinkText
            iconSource: "qrc:/nebula/resources/chevron.svg"
            iconMirror: VPNLocalizer.isRightToLeft
            backgroundColor: VPNTheme.theme.iconButtonLightBackground
            width: parent.width - VPNTheme.theme.windowMargin
            onClicked: {
                VPNGleanDeprecated.recordGleanEvent("helpContactSupportOpened");
                Glean.sample.helpContactSupportOpened.record();
                getHelpStackView.push("qrc:/ui/screens/getHelp/contactUs/ViewContactUsForm.qml");
            }
        }

        VPNExternalLinkListItem {
            objectName: "viewLogs"

            accessibleName: VPNI18n.GetHelpViewLogs
            title: VPNI18n.GetHelpViewLogs
            iconSource: VPNFeatureList.get("shareLogs").isSupported ? "qrc:/nebula/resources/externalLink.svg" : "qrc:/nebula/resources/chevron.svg"
            iconMirror: !VPNFeatureList.get("shareLogs").isSupported && VPNLocalizer.isRightToLeft
            backgroundColor: VPNFeatureList.get("shareLogs").isSupported ?VPNTheme.theme.clickableRowBlue : VPNTheme.theme.iconButtonLightBackground
            width: parent.width - VPNTheme.theme.windowMargin
            onClicked: {
                VPNGleanDeprecated.recordGleanEvent("helpMenuViewLogsOpened");
                Glean.sample.helpMenuViewLogsOpened.record();
                VPN.requestViewLogs()
            }
        }

        VPNExternalLinkListItem {
            objectName: "settingsGiveFeedback"

            accessibleName: title
            title: qsTrId("vpn.settings.giveFeedback")
            onClicked: getHelpStackView.push("qrc:/ui/screens/getHelp/giveFeedback/ViewGiveFeedback.qml")
            iconSource: "qrc:/nebula/resources/chevron.svg"
            iconMirror: VPNLocalizer.isRightToLeft
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
            imageRightMirror: VPNLocalizer.isRightToLeft
            visible: VPNSettings.developerUnlock
            onClicked: getHelpStackView.push("qrc:/ui/screens/getHelp/developerMenu/ViewDeveloperMenu.qml")
        }

    }
}
