/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1

FocusScope {
    property var _filterProxyCallback: () => {}
    property var _sortProxyCallback: () => {}
    property var _editCallback: () => {}
    property alias _filterProxySource: model.source
    property alias _searchBarPlaceholderText: searchBar._placeholderText
    property bool _searchBarHasError: false
    readonly property bool isEmpty: searchBar.length === 0
    property var _getNextTabItem: () => { return null; }

    implicitHeight: searchBarColumn.implicitHeight
    implicitWidth: searchBarColumn.implicitWidth

    ColumnLayout {
        id: searchBarColumn

        anchors.fill: parent
        spacing: MZTheme.theme.windowMargin / 2

        MZTextField {
            id: searchBar
            objectName: "searchBarTextField"

            Accessible.editable: false
            Accessible.searchEdit: true
            Layout.fillWidth: true
            focus: true

            background: MZInputBackground {}
            leftInset: MZTheme.theme.windowMargin * 3
            leftPadding: MZTheme.theme.windowMargin * 3
            rightPadding: MZTheme.theme.windowMargin * 3
            rightInset: MZTheme.theme.windowMargin * 3
            hasError: _searchBarHasError

            onLengthChanged: text => model.invalidate()
            onTextChanged: {
                if (activeFocus) {
                    _editCallback();
                }
            }

            function handleTabPressed() {
                let nextTabItem = _getNextTabItem();
                if (!nextTabItem) {
                    nextTabItem = searchBar.nextItemInFocusChain();
                }
                nextTabItem.forceActiveFocus(Qt.TabFocusReason);
            }

            Keys.onTabPressed: handleTabPressed()

            MZIcon {
                anchors {
                    left: parent.left
                    leftMargin: MZTheme.theme.hSpacing
                    verticalCenter: parent.verticalCenter
                }
                source: "qrc:/nebula/resources/search.svg"
                sourceSize.height: MZTheme.theme.windowMargin
                sourceSize.width: MZTheme.theme.windowMargin
                opacity: parent.focus ? 1 : 0.8
            }
        }

        MZContextualAlerts {
            id: searchWarning
            objectName: "searchBarError"
            Layout.fillWidth: true
            visible: _searchBarHasError

            messages: [
                {
                    type: "error",
                    message: MZI18n.ServersViewSearchNoResultsLabel,
                    visible: searchBar.hasError
                }
            ]
        }

        MZFilterProxyModel {
            id: model
            filterCallback: _filterProxyCallback
            sortCallback: _sortProxyCallback
        }
    }

    function getProxyModel() {
        return model;
    }

    function getSearchBarText() {
        return searchBar.text.toLowerCase();
    }

    function clearText() {
        searchBar.text = "";
    }
}
