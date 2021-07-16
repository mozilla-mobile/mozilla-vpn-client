/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */


import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.14

import Mozilla.VPN 1.0
import "../themes/themes.js" as Theme
import "forms"

ColumnLayout {
    id: appListContainer

    property var header: ""
    property string searchBarPlaceholder: ""

    anchors.left: parent.left
    anchors.right:parent.right
    anchors.leftMargin: Theme.vSpacing
    anchors.rightMargin: Theme.vSpacing
    spacing: Theme.vSpacing

    states: [
        State {
            name: "visibleAndEnabled"
            when: VPNSettings.protectSelectedApps && vpnFlickable.vpnIsOff
            PropertyChanges {
                target: appListContainer
                opacity: 1
                visible: true
            }

        },
        State {
            when: VPNSettings.protectSelectedApps && !vpnFlickable.vpnIsOff
            PropertyChanges {
                target: appListContainer
                opacity: .5
                visible: true
            }
        },
        State {
            name: "hidden"
            when: !VPNSettings.protectSelectedApps
            PropertyChanges {
                target: appListContainer
                opacity: 0
                visible: false
            }
        }
    ]

    transitions: [
        Transition {
            to: "hidden"
            SequentialAnimation {
                PropertyAnimation {
                    target: appListContainer
                    property: "opacity"
                    to: 0
                    duration: 150
                }
                PropertyAction {
                    target: appListContainer
                    property: "visible"
                    value: false
                }
            }
        },
        Transition {
            to: "visibleAndEnabled"
            SequentialAnimation {
                PropertyAction {
                    target: appListContainer
                    property: "visible"
                    value: true
                }
                PropertyAnimation {
                    target: appListContainer
                    property: "opacity"
                    from: 0
                    to: 1
                    duration: 150
                }
            }
        }
    ]

    VPNBoldLabel {
        id: label
        text: header
        Accessible.role: Accessible.Heading
        color: Theme.fontColorDark
        horizontalAlignment: Text.AlignLeft
        Layout.alignment: Qt.AlignLeft
        Layout.topMargin: 4
        verticalAlignment: Text.AlignVCenter
        lineHeight: Theme.vSpacing
        lineHeightMode: Text.FixedHeight
        wrapMode: Text.WordWrap
        Layout.preferredWidth: parent.width
    }

    VPNSearchBar {
        id: filterInput
        Layout.fillWidth: true
        Layout.preferredHeight: Theme.rowHeight
        onTextChanged: text => {
            model.invalidate();
        }
        placeholderText: searchBarPlaceholder
        stateError: applist.count === 0
        enabled: vpnFlickable.vpnIsOff && VPNSettings.protectSelectedApps
    }

    VPNFilterProxyModel {
        id: model
        source: VPNAppPermissions
        // No filter
        filterCallback: obj => {
           const filterValue = filterInput.text.toLowerCase();
           return obj.appName.toLowerCase().includes(filterValue);
        }
    }

    ColumnLayout {
        spacing: Theme.windowMargin / 2

        VPNVerticalSpacer {
            Layout.preferredHeight: 1
        }

        Repeater {
            id: applist
            model: model
            delegate: VPNCheckBoxRow {
                showDivider: false
                labelText: appName
                subLabelText: appID
                showAppImage: true
                onClicked: VPNAppPermissions.flip(appID)
                isChecked: !appIsEnabled
                isEnabled: vpnFlickable.vpnIsOff && VPNSettings.protectSelectedApps
                Layout.minimumHeight: Theme.rowHeight * 1.5
            }
        }
        VPNButton {
            text: ""
            Layout.fillWidth: true
            onClicked: VPNAppPermissions.openFilePicker()
            visible: Qt.platform.os === "windows"
            contentItem: Text {
                // for accessibility
                text: addApplication
                color: "transparent"
            }

            RowLayout {
                anchors.centerIn: parent
                VPNIcon {
                    source: "../resources/plus.svg"
                    sourceSize.height: Theme.windowMargin
                    sourceSize.width: Theme.windowMargin
                }
                VPNBoldLabel {
                    text: addApplication
                    color: "white"
                }
            }
        }
    }


}
