/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import Mozilla.VPN.qmlcomponents 1.0
import components 0.1

import org.mozilla.Glean 0.30
import telemetry 0.30


Item {
    id: root

    VPNMenu {
        id: menu
        // Do not translate this string!
        title: "Feature List"
    }

    VPNFilterProxyModel {
        id: flippableOnFeatureModel
        source: VPNFeatureList
        filterCallback: entry => entry.feature.flippableOn && !entry.feature.isSupportedIgnoringFlip
    }

    VPNFilterProxyModel {
        id: flippableOffFeatureModel
        source: VPNFeatureList
        filterCallback: entry => entry.feature.flippableOff && entry.feature.isSupportedIgnoringFlip
    }

    VPNFlickable {
        id: vpnFlickable
        flickContentHeight: featureListHolder.height + 100
        anchors.top: menu.bottom
        height: root.height - menu.height
        anchors.left: parent.left
        anchors.right: parent.right

        ColumnLayout {
            anchors.top:  parent.top
            anchors.left:  parent.left
            anchors.leftMargin: VPNTheme.theme.windowMargin

            spacing: VPNTheme.theme.windowMargin
            id: featureListHolder

            VPNBoldLabel{
                // Do not translate this string!
                text: "Features that can be flipped on"
            }

            Repeater {
                model: flippableOnFeatureModel
                delegate: VPNCheckBoxRow {
                    showDivider: false
                    labelText: feature.name
                    subLabelText: feature.id
                    showAppImage: false
                    onClicked: VPNFeatureList.toggleForcedEnable(feature.id)
                    enabled: true
                    isChecked: feature.isFlippedOn()
                    Layout.minimumHeight: VPNTheme.theme.rowHeight * 1.5
                }
            }

            VPNBoldLabel{
                // Do not translate this string!
                text: "Features that can be flipped off"
            }

            Repeater {
                model: flippableOffFeatureModel
                delegate: VPNCheckBoxRow {
                    showDivider: false
                    labelText: feature.name
                    subLabelText: feature.id
                    showAppImage: false
                    onClicked: VPNFeatureList.toggleForcedDisable(feature.id)
                    enabled: true
                    isChecked: feature.isFlippedOff()
                    Layout.minimumHeight: VPNTheme.theme.rowHeight * 1.5
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
                    Layout.minimumHeight: VPNTheme.theme.rowHeight * 1.5
                }
            }
        }
    }
}
