/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */


import QtQuick 2.5
import QtQuick.Controls
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import Mozilla.VPN.qmlcomponents 1.0
import components.forms 0.1
import components 0.1

ColumnLayout {
    id: appListContainer
    objectName: "appListContainer"
    property string searchBarPlaceholder: ""

    spacing: VPNTheme.theme.vSpacing

    VPNSearchBar {
        property bool sorted: false;
        id: searchBarWrapper
        _filterProxySource: VPNAppPermissions
        _filterProxyCallback: obj => {
             const filterValue = getSearchBarText();
             return obj.appName.toLowerCase().includes(filterValue);
         }
        _searchBarHasError: applist.count === 0
        _searchBarPlaceholderText: searchBarPlaceholder
        Layout.fillWidth: true
    }

    ColumnLayout {
        id: col2
        objectName: "appExclusionsList"

        spacing: VPNTheme.theme.windowMargin / 2
        Layout.fillWidth: true

        VPNLinkButton {
            property int numDisabledApps: VPNSettings.vpnDisabledApps.length

            id: clearAllButton
            objectName: "clearAll"

            Layout.alignment: Qt.AlignLeft

            // Hack to horizontally align the text
            // with column of checkboxes. This is
            // repeated on L132
            Layout.leftMargin: -4

            textAlignment: Text.AlignLeft
            labelText: VPNI18n.SettingsAppExclusionClearAllApps
            fontSize: VPNTheme.theme.fontSize
            fontName: VPNTheme.theme.fontInterSemiBoldFamily

            onClicked: VPNAppPermissions.protectAll();
            enabled: VPNSettings.vpnDisabledApps.length > 0
            visible: applist.count > 0
        }

        Repeater {
            id: applist

            objectName: "appList"
            model: searchBarWrapper.getProxyModel()
            delegate: RowLayout {
                property int idxForFunctionalTests: index
                id: appRow

                objectName: `${appID}-row`
                spacing: VPNTheme.theme.windowMargin
                Layout.preferredHeight: VPNTheme.theme.navBarTopMargin

                function handleClick() {
                    VPNAppPermissions.flip(appID)
                }

                VPNCheckBox {
                    id: checkBox
                    objectName: appID + "-checkbox"
                    onClicked: () => appRow.handleClick()
                    checked: !appIsEnabled
                    Layout.alignment: Qt.AlignVCenter
                }

                Rectangle {
                    Layout.preferredWidth: VPNTheme.theme.windowMargin * 2
                    Layout.preferredHeight: VPNTheme.theme.windowMargin * 2
                    Layout.maximumHeight: VPNTheme.theme.windowMargin * 2
                    Layout.maximumWidth: VPNTheme.theme.windowMargin * 2
                    Layout.alignment: Qt.AlignVCenter
                    color: VPNTheme.theme.transparent
                    radius: VPNTheme.theme.cornerRadius

                    Image {
                        sourceSize.width: VPNTheme.theme.windowMargin * 2
                        sourceSize.height: VPNTheme.theme.windowMargin * 2
                        anchors.centerIn: parent
                        asynchronous: true
                        fillMode:  Image.PreserveAspectFit
                        Component.onCompleted: {
                            if (appID !== "") {
                                source = "image://app/"+appID
                            }
                        }
                    }
                }

                VPNInterLabel {
                    id: label
                    Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                    Layout.fillWidth: true
                    text: appName
                    color: VPNTheme.theme.fontColorDark
                    horizontalAlignment: Text.AlignLeft

                    VPNMouseArea {
                        anchors.fill: undefined
                        width: parent.implicitWidth
                        height: parent.implicitHeight
                        propagateClickToParent: false
                        onClicked: () => appRow.handleClick()
                    }
                }
            }
        }

        VPNLinkButton {
            objectName: "addApplication"
            labelText: addApplication
            textAlignment: Text.AlignLeft
            fontSize: VPNTheme.theme.fontSize
            fontName: VPNTheme.theme.fontInterSemiBoldFamily
            onClicked: VPNAppPermissions.openFilePicker()

            // Hack to horizontally align the "+" sign with the
            // column of checkboxes
            Layout.leftMargin: -1

            visible: Qt.platform.os === "windows"
            iconComponent: Component {
                VPNIcon {
                    source: "qrc:/nebula/resources/plus.svg"
                    sourceSize.height: VPNTheme.theme.iconSmallSize
                    sourceSize.width: VPNTheme.theme.iconSmallSize
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }
    }
}
