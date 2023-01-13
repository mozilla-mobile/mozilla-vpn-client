/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import Mozilla.VPN.qmlcomponents 1.0
import components.forms 0.1

ColumnLayout {
    property var _filterProxyCallback: () => {}
    property var _sortProxyCallback: () => {}
    property var _editCallback: () => {}
    property alias _filterProxySource: model.source
    property alias _searchBarPlaceholderText: searchBar._placeholderText
    property bool _searchBarHasError: false

    spacing: VPNTheme.theme.windowMargin / 2

    VPNTextField {
        id: searchBar
        objectName: "searchBarTextField"

        Accessible.editable: false
        Accessible.searchEdit: true
        Layout.fillWidth: true

        background: VPNInputBackground {}
        leftInset: VPNTheme.theme.windowMargin * 3
        leftPadding: VPNTheme.theme.windowMargin * 3
        rightPadding: VPNTheme.theme.windowMargin * 3
        rightInset: VPNTheme.theme.windowMargin * 3
        hasError: _searchBarHasError

        onActiveFocusChanged: if (focus && vpnFlickable.ensureVisible) {
            vpnFlickable.ensureVisible(searchBar);
        }
        onLengthChanged: text => model.invalidate()
        onTextChanged: {
            if (focus) {
                _editCallback();
            }
        }

        VPNIcon {
            anchors {
                left: parent.left
                leftMargin: VPNTheme.theme.hSpacing
                verticalCenter: parent.verticalCenter
            }
            source: "qrc:/nebula/resources/search.svg"
            sourceSize.height: VPNTheme.theme.windowMargin
            sourceSize.width: VPNTheme.theme.windowMargin
            opacity: parent.focus ? 1 : 0.8
        }
    }

    VPNContextualAlerts {
        id: searchWarning
        objectName: "searchBarError"
        Layout.fillWidth: true
        visible: _searchBarHasError

        messages: [
            {
                type: "error",
                message: VPNl18n.ServersViewSearchNoResultsLabel,
                visible: searchBar.hasError
            }
        ]
    }

    MZFilterProxyModel {
        id: model
        filterCallback: _filterProxyCallback
        sortCallback: _sortProxyCallback
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
