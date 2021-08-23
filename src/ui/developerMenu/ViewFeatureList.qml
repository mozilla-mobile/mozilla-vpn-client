/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.14
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0
import "../components"
import "../themes/themes.js" as Theme

import org.mozilla.Glean 0.15
import telemetry 0.15


Item {
    id: root

    VPNMenu {
        id: menu
        title: VPNl18n.tr(VPNl18n.SettingsDevFeatureListTitle)
        isSettingsView: false
    }

    VPNFilterProxyModel {
        id: editableFeatureModel
        source: VPNFeatureList
        filterCallback: feature => feature.devModeWriteable
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
                text: VPNl18n.tr(VPNl18n.SettingsDevEditableFeatureList)
            }

            Repeater {
                model: editableFeatureModel
                delegate: VPNCheckBoxRow {
                    showDivider: false
                    labelText: name
                    subLabelText: id
                    showAppImage: false
                    onClicked: VPNFeatureList.devModeFlipFeatureFlag(id)
                    // Only enable the list on features where devModeEnable has any impact
                    enabled: true
                    isChecked: devModeEnabled
                    Layout.minimumHeight: Theme.rowHeight * 1.5
                }
            }

            VPNBoldLabel{
                text: VPNl18n.tr(VPNl18n.SettingsDevAllFeatureList)
            }

            Repeater {
                model: VPNFeatureList
                delegate: VPNCheckBoxRow {
                    showDivider: false
                    labelText: name
                    subLabelText: id
                    showAppImage: false
                    enabled: false
                    isChecked: supported
                    Layout.minimumHeight: Theme.rowHeight * 1.5
                }
            }
        }
    }
}
