/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */


import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import Mozilla.VPN.qmlcomponents 1.0
import components.forms 0.1

ColumnLayout {
    id: appListContainer

    property var header: ""
    property string searchBarPlaceholder: ""

    spacing: VPNTheme.theme.windowMargin

    // Ensure the inital presentation of the component is consistent with the
    // "hidden" state. This prevents the transition from firing on the
    // component load which appears as a "flicker".
    opacity: 0
    visible: false

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
        color: VPNTheme.theme.fontColorDark
        horizontalAlignment: Text.AlignLeft
        Layout.alignment: Qt.AlignLeft
        verticalAlignment: Text.AlignVCenter
        lineHeight: VPNTheme.theme.vSpacing
        lineHeightMode: Text.FixedHeight
        wrapMode: Text.WordWrap
        Layout.preferredWidth: parent.width
    }

    VPNSearchBar {
        id: searchBar
        _filterProxySource: VPNAppPermissions
        _filterProxyCallback: obj => {
             const filterValue = getSearchBarText();
             return obj.appName.toLowerCase().includes(filterValue);
         }
        _searchBarHasError: () => { return applist.count === 0 }
        _searchBarPlaceholderText: searchBarPlaceholder

        enabled: vpnFlickable.vpnIsOff && VPNSettings.protectSelectedApps
        Layout.fillWidth: true
    }


    ColumnLayout {
        id: col2
        spacing: VPNTheme.theme.windowMargin
        Layout.fillHeight: false
        Layout.topMargin: VPNTheme.theme.windowMargin / 2
        Layout.fillWidth: true

        Repeater {
            id: applist
            model: searchBar.getProxyModel()
            Layout.fillHeight: false
            visible: count > 0
            delegate: VPNCheckBoxRow {
                showDivider: false
                labelText: appName
                subLabelText: appID
                showAppImage: true
                onClicked: VPNAppPermissions.flip(appID)
                isChecked: !appIsEnabled
                isEnabled: vpnFlickable.vpnIsOff && VPNSettings.protectSelectedApps
                Layout.minimumHeight: VPNTheme.theme.rowHeight * 1.5
            }
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
            color: VPNTheme.theme.transparent
        }

        RowLayout {
            anchors.centerIn: parent
            VPNIcon {
                source: "qrc:/nebula/resources/plus.svg"
                sourceSize.height: VPNTheme.theme.windowMargin
                sourceSize.width: VPNTheme.theme.windowMargin
            }
            VPNBoldLabel {
                text: addApplication
                color: "white"
            }
        }
    }
}
