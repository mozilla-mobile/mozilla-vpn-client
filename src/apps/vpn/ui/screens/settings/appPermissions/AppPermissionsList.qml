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

    spacing: 24

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

        spacing: VPNTheme.theme.windowMargin
        Layout.fillHeight: false
        Layout.fillWidth: true

        VPNLinkButton {
            property int numDisabledApps: VPNSettings.vpnDisabledApps.length
            id: clearAllButton
            objectName: "clearAll"

            Layout.alignment: Qt.AlignLeft
            Layout.leftMargin: -4
            textAlignment: Text.AlignLeft
            labelText: VPNl18n.GlobalClearAll.arg(VPNSettings.vpnDisabledApps.length)
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
            Layout.fillHeight: false
            delegate: RowLayout {
                property int idxForFunctionalTests: index
                id: appRow

                objectName: `${appID}-row`
                spacing: VPNTheme.theme.windowMargin
                Layout.minimumHeight: VPNTheme.theme.navBarTopMargin

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
                    radius: 4

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

        VPNButton {
            text: ""
            width: undefined
            Layout.preferredWidth: buttonTextAndIcon.implicitWidth + VPNTheme.theme.windowMargin
            Layout.alignment: Qt.AlignLeft
            Layout.fillWidth: false;

            onClicked: VPNAppPermissions.openFilePicker()
            colorScheme: VPNTheme.theme.iconButtonLightBackground
            visible: Qt.platform.os === "windows"
            contentItem: Text {
                // for accessibility
                text: addApplication
                color: VPNTheme.theme.transparent
            }

            RowLayout {
                id: buttonTextAndIcon
                anchors.left: parent.left
                anchors.leftMargin: 3
                anchors.verticalCenter: parent.verticalCenter
                spacing: 4

                VPNIcon {
                    source: "qrc:/nebula/resources/plus.svg"
                    sourceSize.height: 14
                    sourceSize.width: 14
                    Layout.alignment: Qt.AlignVCenter
                }
                VPNBoldInterLabel {
                    text: addApplication
                    color: VPNTheme.theme.blue
                    font.pixelSize: VPNTheme.theme.fontSize
                    verticalAlignment: Qt.AlignVCenter
                    Layout.alignment: Qt.AlignVCenter
                }
            }
        }
    }
}
