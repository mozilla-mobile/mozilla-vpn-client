/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import themes 0.1

import org.mozilla.Glean 0.23
import telemetry 0.23


Item {
    id: root

    VPNMenu {
        id: menu
        // Do not translate this string!
        title: "Feature List"
        isSettingsView: false
    }

    VPNFilterProxyModel {
        id: editableFeatureModel
        source: VPNFeatureList
        filterCallback: entry => entry.feature.devModeWriteable
    }

    VPNFlickable {
        id: vpnFlickable
        property bool vpnIsOff: (VPNController.state === VPNController.StateOff)
        flickContentHeight: featureListHolder.height + 100
        anchors.top: menu.bottom
        height: root.height - menu.height
        anchors.left: parent.left
        anchors.right: parent.right

        ColumnLayout {
            anchors.top:  parent.top
            anchors.left:  parent.left
            anchors.leftMargin: Theme.windowMargin

            spacing: Theme.windowMargin
            id: featureListHolder

            VPNBoldLabel{
                // Do not translate this string!
                text: "Features that can be toggled"
            }

            Repeater {
                model: editableFeatureModel
                delegate: VPNCheckBoxRow {
                    showDivider: false
                    labelText: feature.name
                    subLabelText: feature.id
                    showAppImage: false
                    onClicked: VPNFeatureList.devModeFlipFeatureFlag(feature.id)
                    // Only enable the list on features where devModeEnable has any impact
                    enabled: true
                    isChecked: feature.isDevModeEnabled()
                    Layout.minimumHeight: Theme.rowHeight * 1.5
                }
            }

            VPNBoldLabel{
                // Do not translate this string!
                text: "Feature list"
            }

            Repeater {
                model: VPNFeatureList
                delegate: VPNCheckBoxRow {
                    showDivider: false
                    labelText: feature.name
                    subLabelText: feature.id
                    showAppImage: false
                    enabled: false
                    isChecked: feature.isSupported
                    Layout.minimumHeight: Theme.rowHeight * 1.5
                }
            }
        }
    }
}
