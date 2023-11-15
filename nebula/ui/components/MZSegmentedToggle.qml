/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import Mozilla.Shared 1.0
import compat 0.1

// MZSegmentedToggle
Rectangle {
    id: root

    property alias model: options.model
    property int selectedIndex: 0
    property QtObject selectedSegment
    property var handleSegmentClick: (() => {});
    property bool focusedViaClick: false //To hide focusOutline when not using keyboard navigation

    implicitHeight: MZTheme.theme.rowHeight
    activeFocusOnTab: true
    color: MZTheme.theme.input.highlight
    radius: 24

    onFocusChanged: if(focus) options.itemAt(selectedIndex).focus = true

    onSelectedIndexChanged: {
        selectedSegment = options.itemAt(selectedIndex)
        options.itemAt(root.selectedIndex).focus = true
    }

    Component.onCompleted: {
        selectedSegment = options.itemAt(selectedIndex)
    }

    function anySegmentFocused() {
        for(let i = 0; i < options.count; i++) {
            if(options.itemAt(i).focus === true) {
                return true
            }
        }
        return false
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
        color: MZTheme.theme.white
        radius: parent.radius

        Behavior on anchors.leftMargin {
            PropertyAnimation {
                duration: 100
            }
        }

        Rectangle {
            id: focusOutline

            color: MZTheme.theme.blueButton.focusOutline
            anchors.fill: parent
            anchors.margins: -3
            radius: parent.radius + anchors.margins
            z: -1

            visible: {
                if (root.focusedViaClick) return false
                return root.anySegmentFocused()
            }
        }
    }

    RowLayout {
        objectName: "segmentedToggleBtnLayout"

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

            delegate: MZButtonBase {
                id: segment

                Layout.preferredWidth: slider.implicitWidth
                Layout.fillHeight: true

                objectName: segmentButtonId

                focusPolicy: Qt.NoFocus
                activeFocusOnTab: true

                Accessible.name: (root.selectedIndex === index ? MZI18n.AccessibilitySelectedAndItemName.arg(MZI18n[segmentLabelStringId]) : MZI18n[segmentLabelStringId]) + MZI18n.AccessibilityCurrentIndexFocusedOfTotalItemsInGroup.arg(index + 1).arg(options.count)

                Rectangle {
                    anchors.fill: parent
                    border.width: MZTheme.theme.focusBorderWidth
                    border.color: MZTheme.theme.fontColor
                    color: MZTheme.theme.transparent
                    visible: parent.focus
                    radius: root.radius
                }

                onFocusChanged: {
                    if(!root.anySegmentFocused()) root.focusedViaClick = false
                }

                Keys.onLeftPressed: {
                    if(root.selectedIndex !== 0) {
                        root.focusedViaClick = false
                        options.itemAt(root.selectedIndex).focus = true
                        root.selectedIndex--
                        root.handleSegmentClick(options.itemAt(root.selectedIndex))
                    }
                }

                Keys.onRightPressed: {
                    if(root.selectedIndex !== options.count - 1) {
                        root.focusedViaClick = false
                        options.itemAt(root.selectedIndex).focus = true
                        root.selectedIndex++
                        root.handleSegmentClick(options.itemAt(root.selectedIndex))
                    }
                }

                Loader {
                    anchors.fill: parent
                    anchors.leftMargin: 12
                    anchors.rightMargin: 12
                    sourceComponent: typeof segmentIconPath !== "undefined" ? iconLabel : textLabel
                }

                Component {
                    id: textLabel

                    Text {
                        id: text

                        anchors.centerIn: parent
                        text: MZI18n[segmentLabelStringId]
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        // Bug VPN-2158 - Apparently QText is not rendered on android
                        // When we select any other elide then none
                        elide: Qt.platform.os === "android" ? Text.ElideNone : Text.ElideRight
                        font.family: MZTheme.theme.fontBoldFamily
                        font.pixelSize: MZTheme.theme.fontSize
                        Accessible.ignored: !visible
                        color: {
                            if (root.selectedIndex === index) {
                                return MZTheme.colors.purple70
                            }

                            switch(segment.state) {
                            case MZTheme.theme.uiState.statePressed:
                                return MZTheme.colors.purple70
                            case MZTheme.theme.uiState.stateHovered:
                                return MZTheme.theme.fontColorDark
                            default:
                                return MZTheme.colors.grey40
                            }
                        }

                        Behavior on color {
                            PropertyAnimation {
                                duration: 100
                            }
                        }
                    }
                }

                Component {
                    id: iconLabel

                    MZIcon {
                        source: index === root.selectedIndex ? selectedSegmentIconPath : segmentIconPath
                    }
                }

                onClicked: {
                    if(root.selectedIndex !== index) {
                        root.focusedViaClick = true
                        root.selectedIndex = index
                        root.handleSegmentClick(segment)
                    }
                }

                MZMouseArea {
                    id: mouseArea
                }
            }
        }
    }
}
