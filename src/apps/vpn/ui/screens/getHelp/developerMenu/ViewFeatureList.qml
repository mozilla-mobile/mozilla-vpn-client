/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import Mozilla.VPN.qmlcomponents 1.0
import components 0.1

import org.mozilla.Glean 0.30
import telemetry 0.30


MZViewBase {
    _menuTitle: "Feature list"
    _viewContentData: ColumnLayout {
        id: featureListHolder
        spacing: MZTheme.theme.windowMargin
        Layout.fillWidth: true
        Layout.leftMargin: MZTheme.theme.windowMargin * 1.5
        Layout.rightMargin: MZTheme.theme.windowMargin * 1.5

        Repeater {
            id: rep

            model: VPNFeatureList
            delegate: ColumnLayout {
                Layout.fillWidth: true
                spacing: MZTheme.theme.windowMargin / 1.5

                RowLayout {

                    Layout.fillWidth: true

                    ColumnLayout {
                        Layout.fillWidth: true

                        MZLightLabel {
                            text: feature.name
                            color: MZTheme.theme.fontColorDark
                        }

                        MZTextBlock {
                            text: `id: ${feature.id}`
                            font.pixelSize: MZTheme.theme.fontSizeSmall
                        }
                    }

                    Rectangle {
                        Layout.preferredHeight: 1
                        Layout.fillWidth: true
                        color: MZTheme.theme.transparent
                    }

                    MZSettingsToggle {
                        accessibleName: feature.name
                        checked: feature.isSupported
                        enabled: feature.isToggleable
                        Layout.preferredHeight: 24
                        Layout.preferredWidth: 45
                        Layout.alignment: Qt.AlignTop | Qt. AlignRight
                        opacity: enabled ? 1 : .3
                        onClicked: {
                            VPNFeatureList.toggle(feature.id);
                        }
                    }

                }
                Rectangle {
                    Layout.alignment: Qt.AlignBottom
                    Layout.fillWidth: true
                    Layout.preferredHeight: 1
                    color: MZTheme.colors.grey10
                    visible: index < rep.count - 1
                }
            }
        }

        MZVerticalSpacer {
            Layout.preferredHeight: 1
            Layout.fillWidth: true
        }
    }
}
