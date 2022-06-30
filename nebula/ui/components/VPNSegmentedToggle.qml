/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import Mozilla.VPN 1.0

// VPNSegmentedToggle
Rectangle {
    id: root

    property alias model: options.model
    property int selectedIndex: 0
    property QtObject selectedSegment
    property var handleSegmentClick: (() => {});

    Keys.onLeftPressed: {
        if(root.selectedIndex !== 0) {
            root.selectedIndex--
            root.handleSegmentClick(options.itemAt(root.selectedIndex))
        }
    }

    Keys.onRightPressed: {
        if(root.selectedIndex !== options.count - 1) {
            root.selectedIndex++
            root.handleSegmentClick(options.itemAt(root.selectedIndex))
        }
    }

    implicitHeight: VPNTheme.theme.rowHeight
    activeFocusOnTab: true
    color: VPNTheme.theme.input.highlight
    radius: 24

    onSelectedIndexChanged: {
        selectedSegment = options.itemAt(selectedIndex)
    }

    Component.onCompleted: {
        selectedSegment = options.itemAt(selectedIndex)
    }

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

        VPNFocusOutline {
            visible: root.activeFocus
            focusedComponent: root
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

            delegate: VPNButtonBase {
                id: segment

                Layout.preferredWidth: slider.implicitWidth
                Layout.fillHeight: true

                objectName: segmentButtonId

                focusPolicy: Qt.NoFocus

                Accessible.description: VPNl18n.AccessibilityCurrentIndexFocusedOfTotalItemsInGroup.arg(index + 1).arg(options.count) //`${index + 1} ${VPNl18n.AccessiblityOf} ${options.count}`
                Accessible.name: root.selectedIndex === index ? VPNl18n.AccessibilitySelectedAndItemName.arg(label.text) : label.text

                contentItem: Text {
                    id: label

                    text: VPNl18n[segmentLabelStringId]
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    // Bug VPN-2158 - Apparently QText is not rendered on android
                    // When we select any other elide then none
                    elide: Qt.platform.os === "android" ? Text.ElideNone : Text.ElideRight
                    font.family: VPNTheme.theme.fontBoldFamily
                    font.pixelSize: VPNTheme.theme.fontSize
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

                    Behavior on color {
                        PropertyAnimation {
                            duration: 100
                        }
                    }
                }

                onClicked: {
                    if(root.selectedIndex !== index) {
                        root.selectedIndex = index
                        root.handleSegmentClick(segment)
                    }
                }

                VPNMouseArea {
                    id: mouseArea
                }
            }
        }
    }
}
