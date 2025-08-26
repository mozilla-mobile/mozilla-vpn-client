/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */


import QtQuick 2.5
import QtQuick.Controls
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components.forms 0.1
import components 0.1
import "qrc:/nebula/utils/MZAssetLookup.js" as MZAssetLookup

// AppPermissionsList displays the list of installed applications, allowing users to exclude applications from VPN tunneling
// (split tunneling). It uses a ListView (MZList) for better performance, because that control supports lazy loading of list items.
// The ListView header contains controls preceding the list items, such as the Search Bar and 'Clear All' buttons. The footer
// contains controls appearing after the list items, such as the 'Add Application' button.
//
// The ListView lazily generates list items when they come into view, so the order of creation is unpredictable. To handle this,
// activeFocusOnTab is disabled on the list item checkbox, and tabbing between list items is implemented by making
// Tab and Backtab to navigate by list item index.

ColumnLayout {
    id: appListContainer
    objectName: "appListContainer"
    property string searchBarPlaceholder: ""
    property int availableHeight: 0;
    property bool showSystemApps: false;
    property string searchQuery: "";

    MZFilterProxyModel {
        id: showSystemAppsModel
        source:  VPNAppPermissions
        filterCallback: obj => { 
            if (!appListContainer.showSystemApps && obj.isSystemApp) {
                return false;
            }
            return obj.appName.toLowerCase().includes(appListContainer.searchQuery.toLowerCase());
        }
    }

    // ListView Header
    Component {
        id: appListHeader

        FocusScope {
            id: appListHeaderFocusScope

            readonly property ListView listView: ListView.view
            readonly property var getProxyModel: showSystemAppsModel

            implicitHeight: appListHeaderColumn.implicitHeight
            implicitWidth: appListHeaderColumn.implicitWidth

            ColumnLayout {
                id: appListHeaderColumn

                spacing: MZTheme.theme.vSpacingSmall
                width: listView.width - MZTheme.theme.vSpacing

                Loader {
                    Layout.fillWidth: true

                    active: Qt.platform.os === "linux" && VPNController.state !== VPNController.StateOff
                    visible: active

                    sourceComponent: MZInformationCard {
                        width: parent.width
                        cardType: MZInformationCard.CardType.Info
                        implicitHeight: textBlock.height + MZTheme.theme.windowMargin * 2
                        anchors.top: parent.top
                        anchors.topMargin: MZTheme.theme.viewBaseTopMargin

                        _infoContent: MZTextBlock {
                            id: textBlock
                            Layout.fillWidth: true

                            text: MZI18n.SplittunnelInfoCardDescription2
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }

                MZVerticalSpacer {
                    height: MZTheme.theme.dividerHeight
                }

                MZSearchBar {
                    property bool sorted: false;
                    id: searchBarWrapper
                    _editCallback: () => {
                        // Clear the list selection when editing the SearchBar to prevent the focus from moving
                        // from the SearchBar to the selected list item.
                        listView.currentIndex = -1; 
                        appListContainer.searchQuery = getSearchBarText();
                        showSystemAppsModel.recalculate();
                    }
                    // Return next item to be tabbed to
                    _getNextTabItem: () => {
                        if (clearAllButton.enabled) {
                            return clearAllButton;
                        } else if (listView.count > 0) {
                            // First item in the list
                            listView.currentIndex = 0;
                            return listView.itemAtIndex(0);
                        } else {
                            // Next item in focus order
                            return null;
                        }
                    }
                    _searchBarHasError: listView.count === 0
                    _searchBarPlaceholderText: searchBarPlaceholder
                    Layout.fillWidth: true
                }

                MZLinkButton {
                    property int numDisabledApps: MZSettings.vpnDisabledApps.length

                    id: clearAllButton
                    objectName: "clearAll"

                    Layout.alignment: Qt.AlignLeft

                    // Hack to horizontally align the text
                    // with column of checkboxes.
                    Layout.leftMargin: -4

                    textAlignment: Text.AlignLeft
                    labelText: MZI18n.SettingsAppExclusionClearAllApps
                    fontSize: MZTheme.theme.fontSize
                    fontName: MZTheme.theme.fontInterSemiBoldFamily
                    focus: enabled

                    onClicked: {
                        VPNAppPermissions.protectAll();
                    }
                    enabled: MZSettings.vpnDisabledApps.length > 0
                    visible: listView.count > 0

                    function handleTabPressed() {
                        if (listView.count > 0) {
                            listView.currentIndex = 0;
                            listView.currentItem.forceActiveFocus(Qt.TabFocusReason);
                        } else {
                            navbar.nextItemInFocusChain().forceActiveFocus(Qt.TabFocusReason);
                        }
                    }

                    Keys.onTabPressed: handleTabPressed()
                }

                MZVerticalSpacer {
                    height: MZTheme.theme.dividerHeight
                }
            }
        }
    }

    MZList {
        id: appList

        objectName: "appList"
        model: headerItem.getProxyModel
        height: availableHeight
        Layout.fillWidth: true
        spacing: MZTheme.theme.windowMargin

        delegate: FocusScope {
            id: appRowFocusScope

            readonly property ListView listView: ListView.view

            implicitHeight: appRow.implicitHeight
            implicitWidth: appRow.implicitWidth

            RowLayout {
                id: appRow

                property string appIdForFunctionalTests: appID

                objectName: `app-${index}`
                spacing: MZTheme.theme.windowMargin
                opacity: enabled ? 1.0 : 0.5
                Layout.preferredHeight: MZTheme.theme.navBarTopMargin

                function handleClick() {
                    VPNAppPermissions.flip(appID)
                }

                MZCheckBox {
                    id: checkBox
                    objectName: "checkbox"
                    onClicked: () => appRow.handleClick()
                    checked: !appIsEnabled
                    // Disable activeFocusOnTab as it relies on the order of element creation. The ListView lazily generates
                    // list items when they come into view, so the order of creation is unpredictable. Instead, this
                    // checkbox manages Tab/Backtab to navigate by list item index.
                    checkBoxActiveFocusOnTab: false
                    Layout.alignment: Qt.AlignVCenter
                    Accessible.name: appName
                    focus: true

                    // Change list selection on focus change
                    onActiveFocusChanged: {
                        if (activeFocus) { 
                            listView.currentIndex = index;
                        };
                    }

                    function handleTabPressed() {
                        if (listView.currentIndex < (listView.count - 1)) {
                            // Move selection & focus to next item
                            listView.incrementCurrentIndex();
                            listView.currentItem.forceActiveFocus(Qt.TabFocusReason);
                        }
                        else {
                            // Currently at end of list. Move focus to footer
                            listView.footerItem.forceActiveFocus(Qt.TabFocusReason);
                        }
                    }

                    function handleBacktabPressed() {
                        if (listView.currentIndex > 0) {
                            // Move selection & focus to previous item
                            listView.decrementCurrentIndex();
                            listView.currentItem.forceActiveFocus(Qt.BacktabFocusReason);
                        }
                        else {
                            // Currently at top of list. Move focus to header
                            listView.headerItem.forceActiveFocus(Qt.BacktabFocusReason);
                        }
                    }

                    Keys.onTabPressed: handleTabPressed()
                    Keys.onBacktabPressed: handleBacktabPressed()
                }

                Rectangle {
                    Layout.preferredWidth: MZTheme.theme.windowMargin * 2
                    Layout.preferredHeight: MZTheme.theme.windowMargin * 2
                    Layout.maximumHeight: MZTheme.theme.windowMargin * 2
                    Layout.maximumWidth: MZTheme.theme.windowMargin * 2
                    Layout.alignment: Qt.AlignVCenter
                    color: MZTheme.colors.transparent

                    Image {
                        height: MZTheme.theme.windowMargin * 2
                        width: MZTheme.theme.windowMargin * 2
                        sourceSize.width: MZTheme.theme.windowMargin * 2
                        sourceSize.height: MZTheme.theme.windowMargin * 2
                        anchors.centerIn: parent
                        fillMode:  Image.PreserveAspectFit
                        Component.onCompleted: {
                            if (appID !== "") {
                                source = "image://app/"+appID
                            }
                        }
                        
                    }
                }

                MZInterLabel {
                    id: label
                    Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                    Layout.fillWidth: true
                    text: appName
                    color: MZTheme.colors.fontColorDark
                    horizontalAlignment: Text.AlignLeft
                }

                MZMouseArea {
                    anchors.fill: parent
                    width: parent.implicitWidth
                    height: parent.implicitHeight
                    propagateClickToParent: false
                    onClicked: () => appRow.handleClick()
                }
            }
        }

        header: appListHeader
        footer: appListFooter

        Component.onCompleted: {
            appList.positionViewAtBeginning();

            // Sometimes appList is not scrolled to the beginning despite the previous request. Try again after
            // a delay to allow for layout to complete.
            scrollTimer.setTimeout(function() {
                appList.positionViewAtBeginning();
                }, 10);
        }

        // Restore scroll position, selected item and focus when the model changes
        Connections {
            target: appList.model

            property Item previousFocusItem: null
            property int previousIndex: -1
            property real diffY: -1

            function onModelAboutToBeReset() {
                previousFocusItem = window.activeFocusItem;
                previousIndex = appList.currentIndex;
                diffY = appList.contentY - appList.originY;
            }

            function onModelReset() {
                appList.currentIndex = previousIndex;
                previousFocusItem.forceActiveFocus();
                appList.contentY =  appList.originY + diffY;
            }

            function onDataChanged() {
                 appList.currentIndex = previousIndex;
                 previousFocusItem.forceActiveFocus();
                 appList.contentY =  appList.originY + diffY;
            }
        }
    }

    // ListView Footer
    Component {
        id: appListFooter

        FocusScope
        {
            id: appListFooterFocusScope
            readonly property ListView listView: ListView.view

            implicitHeight: appListFooterColumn.implicitHeight
            implicitWidth: appListFooterColumn.implicitWidth

            ColumnLayout {
                id: appListFooterColumn

                spacing: MZTheme.theme.vSpacingSmall
                width: listView.width - MZTheme.theme.vSpacing

                MZVerticalSpacer {
                    height: MZTheme.theme.dividerHeight
                }

                MZLinkButton {
                    objectName: "addApplication"
                    id: addApp
                    labelText: addApplication
                    textAlignment: Text.AlignLeft
                    fontSize: MZTheme.theme.fontSize
                    fontName: MZTheme.theme.fontInterSemiBoldFamily
                    focus: true
                    onClicked: {
                        VPNAppPermissions.openFilePicker()
                    }

                    function handleBacktabPressed() {
                        // Move focus back to last item in list
                        if (listView.count > 0) {
                            listView.currentIndex = listView.count - 1;
                            listView.currentItem.forceActiveFocus(Qt.BacktabFocusReason);
                        }
                    }

                    Keys.onTabPressed: navbar.nextItemInFocusChain().forceActiveFocus(Qt.TabFocusReason);
                    Keys.onBacktabPressed: handleBacktabPressed()

                    // Hack to horizontally align the "+" sign with the
                    // column of checkboxes
                    Layout.leftMargin: -1

                    visible: Qt.platform.os === "windows"
                    iconComponent: Component {
                        MZIcon {
                            source: MZAssetLookup.getImageSource("Plus")
                            sourceSize.height: MZTheme.theme.iconSizeSmall
                            sourceSize.width: MZTheme.theme.iconSizeSmall
                            anchors.bottom: parent.bottom
                            anchors.right: parent.right
                            anchors.rightMargin: -2
                            anchors.bottomMargin: 1
                        }
                    }
                }

                RowLayout {
                    visible: VPNAppPermissions.containsSystemApps 
                    spacing: MZTheme.theme.windowMargin
                    opacity: enabled ? 1.0 : 0.5
                    Layout.preferredHeight: MZTheme.theme.navBarTopMargin

                    MZCheckBox {
                        Layout.alignment: Qt.AlignVCenter
                        objectName: "systemCheckbox"
                        onClicked: () => {
                            appListContainer.showSystemApps = !appListContainer.showSystemApps
                            showSystemAppsModel.recalculate();
                        }
             
                        checked: appListContainer.showSystemApps
                    }

                    MZInterLabel {
                        id: label
                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        text: MZI18n.SplittunnelBtnShowSystemApp
                        color: MZTheme.colors.fontColorDark
                        horizontalAlignment: Text.AlignLeft
                    }

                }
                
                // Spacer to allow end of list to scroll up above the navbar
                MZVerticalSpacer {
                    height: MZTheme.theme.navBarHeightWithMargins 
                }
            }
        }
    }

    MZTimer {
        id: scrollTimer
    }
}
