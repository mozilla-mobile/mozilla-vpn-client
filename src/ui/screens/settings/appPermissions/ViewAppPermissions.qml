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

MZViewBase {
    id: vpnFlickable
    objectName: "appPermissions"

    //% "Search apps"
    //: Search bar placeholder text
    property string searchApps: qsTrId("vpn.protectSelectedApps.searchApps")

    //% "Add application"
    //: Button label
    property string addApplication: qsTrId("vpn.protectSelectedApps.addApplication")

    property Component rightMenuButton: Component {
        Loader {
            active: MZFeatureList.get("helpSheets").isSupported
            sourceComponent: MZIconButton {
                objectName: "excludedAppsHelpButton"

                onClicked: helpSheet.active = true

                accessibleName: MZI18n.GetHelpLinkTitle

                Image {
                    anchors.centerIn: parent

                    source: "qrc:/nebula/resources/question.svg"
                    fillMode: Image.PreserveAspectFit
                }
            }
        }
    }

    _menuTitle: MZI18n.SettingsAppExclusionSettings
    _viewContentData: ColumnLayout {

        Layout.preferredWidth: parent.width

        Loader {
            Layout.leftMargin: MZTheme.theme.windowMargin
            Layout.rightMargin: MZTheme.theme.windowMargin
            Layout.bottomMargin: 24
            Layout.fillWidth: true

            active: Qt.platform.os === "linux" && VPNController.state !== VPNController.StateOff
            visible: active

            sourceComponent: MZInformationCard {
                width: parent.width
                implicitHeight: textBlock.height + MZTheme.theme.windowMargin * 2
                _infoContent: MZTextBlock {
                    id: textBlock
                    Layout.fillWidth: true


                    text: MZI18n.SplittunnelInfoCardDescription
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }

        AppPermissionsList {
            id: enabledList

            Layout.fillWidth: true
            Layout.fillHeight: false
            Layout.leftMargin: MZTheme.theme.vSpacing
            Layout.rightMargin: MZTheme.theme.vSpacing

            searchBarPlaceholder: searchApps
            enabled: Qt.platform.os === "linux" ? VPNController.state === VPNController.StateOff : true
        }
    }


    MZHelpSheet {
        id: helpSheet
        objectName: "excludedAppsHelpSheet"

        title: MZI18n.HelpSheetsExcludedAppsTitle

        model: [
            {type: MZHelpSheet.BlockType.Title, text: MZI18n.HelpSheetsExcludedAppsHeader},
            {type: MZHelpSheet.BlockType.Text, text: MZI18n.HelpSheetsExcludedAppsBody1, margin: MZTheme.theme.helpSheetTitleBodySpacing},
            {type: MZHelpSheet.BlockType.Text, text: MZI18n.HelpSheetsExcludedAppsBody2, margin: MZTheme.theme.helpSheetBodySpacing},
            {type: MZHelpSheet.BlockType.Text, text: MZI18n.HelpSheetsExcludedAppsBody3, margin: MZTheme.theme.helpSheetBodySpacing},
            {type: MZHelpSheet.BlockType.PrimaryButton, text: MZI18n.HelpSheetsExcludedAppsCTA, margin: MZTheme.theme.helpSheetBodyButtonSpacing, action: () => {
                    close()
                    getStack().push("qrc:/ui/screens/settings/privacy/ViewPrivacy.qml")
                }, objectName: "openPrivacyFeaturesButton"},
            {type: MZHelpSheet.BlockType.LinkButton, text: MZI18n.GlobalLearnMore, margin: MZTheme.theme.helpSheetSecondaryButtonSpacing, action: () => { MZUrlOpener.openUrlLabel("sumoExcludedApps") }, objectName: "learnMoreLink"},
        ]

        onActiveChanged: if (active) item.open()
    }

    Component.onCompleted: {
        console.log("Component ready");
        VPNAppPermissions.requestApplist();
        Glean.sample.appPermissionsViewOpened.record();
    }
}
