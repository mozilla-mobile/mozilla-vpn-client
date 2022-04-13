/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import Mozilla.VPN 1.0

// VPNSegmentedToggle
Rectangle {
    id: root

    property alias model: options.model
    property int selectedIndex: 0
    property QtObject selectedSegment
    property var handleSegmentClick: (() => {});

    onSelectedIndexChanged: {
        selectedSegment = options.itemAt(selectedIndex)
    }

    Component.onCompleted: {
        selectedSegment = options.itemAt(selectedIndex)
    }

    implicitHeight: 40
    color: VPNTheme.theme.input.highlight
    radius: 24

    Rectangle {
        id: slider
        anchors {
            top: parent.top
            left: parent.left
            bottom: parent.bottom
            topMargin: 4
            leftMargin: implicitWidth * root.selectedIndex + 4
            rightMargin: 4
            bottomMargin: 4
        }

        implicitWidth: (parent.width - 8) / model.count
        color: VPNTheme.theme.white
        radius: parent.radius

        Behavior on anchors.leftMargin {
            PropertyAnimation {
                duration: 100
            }

        }
    }

    RowLayout {

        anchors {
            fill: parent
            topMargin: 8
            leftMargin: 4
            rightMargin: 4
            bottomMargin: 8
        }

        spacing: 0

        Repeater {
            id: options

            delegate: Item {
                id: segment

                Layout.fillWidth: true
                Layout.fillHeight: true

                objectName: segmentButtonId


                Accessible.role: Accessible.Button
                Accessible.description: `${index + 1} of ${options.count}`
                Accessible.name: {
                    var name = ""
                    if(root.selectedIndex === index) name += "selected,"
                    name += label.text
                    return name
                }
                Accessible.onPressAction: segmentClicked()

                VPNMetropolisLabel {
                    id: label
                    anchors.fill: parent

                    text: VPNl18n[segmentLabelStringId]
                    font.family: VPNTheme.theme.fontBoldFamily
                    color: {
                        if (root.selectedIndex === index) {
                            return VPNTheme.colors.purple70
                        }

                        switch(segment.state) {
                        case VPNTheme.theme.uiState.statePressed:
                            return VPNTheme.colors.purple70
                        case VPNTheme.theme.uiState.stateHovered:
                            return VPNTheme.theme.fontColorDark
                        default:
                            return VPNTheme.colors.grey40
                        }
                    }

                    Accessible.ignored: true

                    Behavior on color {
                        PropertyAnimation {
                            duration: 100
                        }
                    }
                }

                VPNMouseArea {
                    propagateClickToParent: false
                    onClicked: segment.segmentClicked()
                }

                function segmentClicked() {
                    if(root.selectedIndex !== index) {
                        root.selectedIndex = index
                        root.handleSegmentClick(segment)
                    }
                }
            }
        }
    }
}
