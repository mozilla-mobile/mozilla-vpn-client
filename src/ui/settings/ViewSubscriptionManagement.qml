/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1

import org.mozilla.Glean 0.30
import telemetry 0.30

VPNFlickable {
    property string _menuTitle: "Profile"

    id: vpnFlickable
    objectName: "settingsView"

    height: parent.height - VPNTheme.theme.menuHeight
    flickContentHeight: contentColumn.implicitHeight + VPNTheme.theme.rowHeight
    windowHeightExceedsContentHeight: !(flickContentHeight > height)

    anchors {
        top: parent.top
        topMargin: VPNTheme.theme.menuHeight
    }

    // TODO: Replace dummy data with subscription details from FxA
    ListModel {
        id: subscriptionSummaryModel
        
        ListElement {
            label: "Subscription plan"
            value: "$4.99 Monthly"
            type: "bold"
        }

        ListElement {
            label: "Status"
            value: "inactive"
            type: "pill"
        }

        ListElement {
            label: "Next billed"
            value: "09/17/2022"
            type: "default"
        }

        ListElement {
            label: "Activated"
            value: "08/17/2022"
            type: "default"
        }

        ListElement {
            label: "Expires"
            value: "08/17/2023"
            type: "default"
        }
    }

    ListModel {
        id: subscriptionPaymentModel
        
        ListElement {
            label: "MasterCard"
            value: "Card ending in 5887"
            type: "default"
            iconUrl: "qrc:/ui/resources/logos/android.svg"
            // iconUrl: "qrc:/ui/resources/logos/apple.svg"
            // iconUrl: "qrc:/ui/resources/logos/mastercard.svg"
        }

        ListElement {
            label: "Expires"
            value: "June 2026"
            type: "default"
        }
    }

    Component {
        id: subscriptionInfoItem

        ColumnLayout {
            spacing: 0
            state: "default"
            states: [
                State {
                    name: "default"
                    when: type === "default"

                    PropertyChanges {
                        target: rowPill
                        visible: false
                    }
                },
                State {
                    name: "bold"
                    when: type === "bold"

                    PropertyChanges {
                        target: rowLabel
                        font.family: VPNTheme.theme.fontInterFamily
                    }

                    PropertyChanges {
                        target: rowPill
                        visible: false
                    }
                },
                State {
                    name: "pill"
                    when: type === "pill"

                    PropertyChanges {
                        target: rowText
                        visible: false
                    }
                }
            ]

            Layout.fillWidth: true

            RowLayout {
                spacing: VPNTheme.theme.listSpacing

                Layout.alignment: Qt.AlignVCenter
                Layout.bottomMargin: VPNTheme.theme.listSpacing
                Layout.fillWidth: true
                Layout.preferredHeight: VPNTheme.theme.rowHeight
                Layout.topMargin: VPNTheme.theme.listSpacing

                VPNInterLabel {
                    id: rowLabel

                    horizontalAlignment: Text.AlignLeft
                    font.pixelSize: VPNTheme.theme.fontSizeSmall
                    text: label
                    wrapMode: Text.WordWrap

                    Layout.fillWidth: true

                    VPNIcon {
                        id: labelIcon

                        source: iconUrl
                        sourceSize.height: VPNTheme.theme.iconSizeSmall * 1.5
                        sourceSize.width: VPNTheme.theme.iconSizeSmall * 1.5
                        visible: typeof(iconUrl) !== "undefined" && source !== ""

                        anchors {
                            left: parent.right
                            verticalCenter: parent.verticalCenter
                        }
                    }
                }

                VPNInterLabel {
                    id: rowText

                    color: VPNTheme.theme.fontColorDark
                    horizontalAlignment: Text.AlignRight
                    font.pixelSize: VPNTheme.theme.fontSizeSmall
                    text: value
                    wrapMode: Text.WordWrap

                    Layout.alignment: Qt.AlignRight
                    Layout.fillWidth: true
                }

                VPNPill {
                    id: rowPill

                    color: value === "active"
                        ? VPNTheme.colors.green90
                        : VPNTheme.colors.red70
                    background: value === "active"
                        ? VPNTheme.colors.green5
                        : VPNTheme.colors.red5
                    text: value === "active" ? "Active" : "Inactive"

                    Layout.alignment: Qt.AlignRight
                }
            }

            Rectangle {
                id: divider

                color: VPNTheme.colors.grey10

                Layout.fillWidth: true
                Layout.leftMargin: 0
                Layout.preferredHeight: 1
                Layout.rightMargin: 0
            }
        }
    }

    ColumnLayout {
        id: contentColumn

        height: Math.max(vpnFlickable.height - VPNTheme.theme.menuHeight, contentColumn.implicitHeight)
        spacing: VPNTheme.theme.windowMargin * 2
        width: parent.width - VPNTheme.theme.windowMargin

        anchors {
            horizontalCenter: parent.horizontalCenter
            top: parent.top
        }

        VPNUserProfile {
            _iconButtonImageSource: "qrc:/nebula/resources/open-in-new.svg"
            _iconButtonOnClicked: () => handleManageAccountClicked()

            Layout.leftMargin: VPNTheme.theme.windowMargin / 2
            Layout.topMargin: VPNTheme.theme.windowMargin * 2
        }

        ColumnLayout {
            spacing: 0

            Layout.leftMargin: VPNTheme.theme.windowMargin / 2
            Layout.rightMargin: VPNTheme.theme.windowMargin / 2

            VPNMetropolisLabel {
                color: VPNTheme.theme.fontColorDark
                horizontalAlignment: Text.AlignLeft
                font.family: VPNTheme.theme.fontBoldFamily
                text: "Subscription summary" // TODO: Add localized strings

                Layout.bottomMargin: VPNTheme.theme.listSpacing * 0.5
                Layout.fillWidth: true
            }

            Repeater {
                id: subscriptionSummaryList

                delegate: subscriptionInfoItem
                model: subscriptionSummaryModel
            }

            VPNMetropolisLabel {
                color: VPNTheme.theme.fontColorDark
                horizontalAlignment: Text.AlignLeft
                font.family: VPNTheme.theme.fontBoldFamily
                text: "Payment information" // TODO: Add localized strings

                Layout.topMargin: VPNTheme.theme.windowMargin * 2
                Layout.bottomMargin: VPNTheme.theme.listSpacing * 0.5
                Layout.fillWidth: true
            }

            Repeater {
                id: subscriptionPaymentList

                delegate: subscriptionInfoItem
                model: subscriptionPaymentModel
            }

            VPNButton {
                id: manageSubscriptionButton

                onClicked: {
                    handleManageAccountClicked();
                }
                text: "Manage subscription" // TODO: Add localized strings
                width: undefined

                Layout.topMargin: VPNTheme.theme.windowMargin * 2
                Layout.fillWidth: true
            }

            VPNLinkButton {
                fontName: VPNTheme.theme.fontBoldFamily
                labelText: VPNl18n.DeleteAccountButtonLabel
                linkColor: VPNTheme.theme.redButton
                visible: VPNFeatureList.get("accountDeletion").isSupported

                onClicked: {
                    settingsStackView.push("qrc:/ui/deleteAccount/ViewDeleteAccount.qml");
                }

                Layout.alignment: Qt.AlignHCenter
                Layout.topMargin: VPNTheme.theme.vSpacing
            }
        }
    }

    function handleManageAccountClicked() {
        Sample.manageAccountClicked.record();
        VPN.openLink(VPN.LinkAccount);
    }
}
