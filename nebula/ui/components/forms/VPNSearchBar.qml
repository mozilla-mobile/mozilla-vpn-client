/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Controls 2.14
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
    property bool _searchBarHasError: false
    property alias _searchBarPlaceholderText: searchBar._placeholderText

    spacing: VPNTheme.theme.windowMargin / 2

    VPNTextField {
        // TODO Add strings for Accessible.description, Accessible.name

        id: searchBar

        background: VPNInputBackground {}
        leftInset: 48
        leftPadding: 48
        onActiveFocusChanged: if (focus && vpnFlickable.ensureVisible) vpnFlickable.ensureVisible(searchBar)
        Layout.fillWidth: true
        onTextChanged: hasError = _searchBarHasError
        onLengthChanged: text => model.invalidate()

        VPNIcon {
            source: "qrc:/nebula/resources/search.svg"
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 20
            sourceSize.height: VPNTheme.theme.windowMargin
            sourceSize.width: VPNTheme.theme.windowMargin
            opacity: parent.focus ? 1 : 0.8
        }


        Keys.onPressed: event => {
            if (focus && _searchBarHasError && (/[\w\[\]`!@#$%\^&*()={}:;<>+'-]/).test(event.text)) {
                _editCallback();
            }
        }
    }

    VPNContextualAlerts {
        id: searchWarning
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

    VPNFilterProxyModel {
        id: model
        filterCallback: _filterProxyCallback
        sortCallback: _sortProxyCallback
    }

    function getProxyModel() {
        return model
    }

    function getSearchBarText() {
        return searchBar.text.toLowerCase();
    }

    function clearText() {
        searchBar.text = ""
    }
}
