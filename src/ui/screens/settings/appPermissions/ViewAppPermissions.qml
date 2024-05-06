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
    // The ListView Flickable in AppPermissionsList is interactive, so prevent conflict by turning off this one
     _interactive: false
    // Turn off top & bottom margins because ListView in AppPermissionsList fills the vertical content area, so additional margins are unecessary
    _useMargins: false

    readonly property string telemetryScreenId : "app_exclusions"

    //% "Search apps"
    //: Search bar placeholder text
    property string searchApps: qsTrId("vpn.protectSelectedApps.searchApps")

    //% "Add application"
    //: Button label
    property string addApplication: qsTrId("vpn.protectSelectedApps.addApplication")

    property Component rightMenuButton: Component {
        Loader {
            active: true
            sourceComponent: MZIconButton {
                objectName: "excludedAppsHelpButton"

                onClicked: {
                    helpSheet.open()

                    Glean.interaction.helpTooltipSelected.record({
                        screen: vpnFlickable.telemetryScreenId,
                    });
                }

                accessibleName: MZI18n.GetHelpLinkText

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
        AppPermissionsList {
            id: enabledList

            Layout.fillWidth: true
            Layout.fillHeight: false
            Layout.leftMargin: MZTheme.theme.vSpacing

            searchBarPlaceholder: searchApps
            enabled: Qt.platform.os === "linux" ? VPNController.state === VPNController.StateOff : true
            availableHeight: window.height - MZTheme.theme.menuHeight
        }
    }

    MZHelpSheet {
        id: helpSheet
        objectName: "excludedAppsHelpSheet"

        property string telemetryScreenId: "app_exclusions_info"

        title: MZI18n.HelpSheetsExcludedAppsTitle

        model: [
            {type: MZHelpSheet.BlockType.Title, text: MZI18n.HelpSheetsExcludedAppsHeader},
            {type: MZHelpSheet.BlockType.Text, text: MZI18n.HelpSheetsExcludedAppsBody1, margin: MZTheme.theme.helpSheetTitleBodySpacing},
            {type: MZHelpSheet.BlockType.Text, text: MZI18n.HelpSheetsExcludedAppsBody2, margin: MZTheme.theme.helpSheetBodySpacing},
            {type: MZHelpSheet.BlockType.Text, text: MZI18n.HelpSheetsExcludedAppsBody3, margin: MZTheme.theme.helpSheetBodySpacing},
            {type: MZHelpSheet.BlockType.PrimaryButton, text: MZI18n.HelpSheetsExcludedAppsCTA, margin: MZTheme.theme.helpSheetBodyButtonSpacing, objectName: "openPrivacyFeaturesButton", action: () => {
                    close()
                    getStack().push("qrc:/ui/screens/settings/privacy/ViewPrivacy.qml")
                }},
            {type: MZHelpSheet.BlockType.LinkButton, text: MZI18n.GlobalLearnMore, margin: MZTheme.theme.helpSheetSecondaryButtonSpacing, objectName: "learnMoreLink", action: () => {
                    MZUrlOpener.openUrlLabel("sumoExcludedApps")
                    Glean.interaction.learnMoreSelected.record({
                        screen: telemetryScreenId
                    });
                }}
        ]

        onOpened: {
            Glean.impression.appExclusionsInfoScreen.record({
                screen: telemetryScreenId,
            });
        }
    }

    Component.onCompleted: {
        VPNAppPermissions.requestApplist();
        Glean.impression.appExclusionsScreen.record({
            screen: telemetryScreenId,
        });
    }
}
