/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Layouts 1.14
import QtQuick.Controls 2.14

import Mozilla.VPN 1.0

Rectangle {
    id: root
    objectName: "vpnCollapsibleCard"

    property int animationDuration: 150
    property bool expanded: false
    property string title: ""
    property string iconSrc: ""
    property alias contentItem: cardContent.data

    color: VPNTheme.colors.white
    implicitHeight: expanded ? cardWrapper.wrapperHeight : accordionHeader.headerHeight
    radius: VPNTheme.theme.cornerRadius
    width: parent.width
    clip: true

    Behavior on implicitHeight {
        NumberAnimation {
            duration: animationDuration
            easing.type: Easing.OutQuad
        }
    }

    VPNDropShadowWithStates {
        anchors.fill: shadowSource
        source: shadowSource
        state: "overwrite-state"
        cached: true
        transparentBorder: true
        z: -1
    }

    Rectangle {
        id: shadowSource
        anchors.fill: root
        radius: root.radius
        color: parent.color
    }

    states: [
        State {
            name: "expanded"
            when: root.expanded

            PropertyChanges {
                target: cardContent
                opacity: 1
            }

            PropertyChanges {
                target: chevron
                rotation: -90
            }
        },
        State {
            name: "collapsed"
            when: !root.expanded

            PropertyChanges {
                target: cardContent
                opacity: 0
            }

            PropertyChanges {
                target: chevron
                rotation: 90
            }
        }
    ]

    ColumnLayout {
        id: cardWrapper

        property int wrapperHeight: implicitHeight + anchors.topMargin + anchors.bottomMargin

        anchors.top: parent.top
        anchors.topMargin: VPNTheme.theme.listSpacing
        anchors.bottomMargin: VPNTheme.theme.listSpacing
        spacing: 0
        width: root.width


        // Card header
        RowLayout {
            id: accordionHeader

            property int headerHeight: implicitHeight + parent.anchors.topMargin + parent.anchors.bottomMargin

            spacing: VPNTheme.theme.listSpacing * 2
            Layout.leftMargin: VPNTheme.theme.listSpacing * 2
            Layout.rightMargin: VPNTheme.theme.listSpacing * 2
            Layout.fillWidth: true

            VPNIcon {
                id: icon

                source: iconSrc
                visible: iconSrc !== ""
            }

            Text {
                id: accordionTitle

                Layout.fillWidth: true
                Layout.preferredWidth: cardWrapper.width

                color: VPNTheme.colors.grey50
                font.family: VPNTheme.theme.fontBoldFamily
                font.pixelSize: VPNTheme.theme.fontSize
                lineHeight: VPNTheme.theme.labelLineHeight
                lineHeightMode: Text.FixedHeight
                text: root.title
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.Wrap

                Accessible.role: Accessible.StaticText
                Accessible.name: text
            }

            VPNIconButton {
                id: stateIndicator
                objectName: "vpnCollapsibleCardChevron"

                onClicked: {
                    handleToggleCard();
                }
                radius: VPNTheme.theme.cornerRadius - 1
                Layout.preferredHeight: VPNTheme.theme.rowHeight
                Layout.preferredWidth: VPNTheme.theme.rowHeight

                accessibleName: expanded ? VPNl18n.GlobalCollapse : VPNl18n.GlobalExpand

                VPNChevron {
                    id: chevron

                    anchors.centerIn: stateIndicator
                    source: "qrc:/nebula/resources/chevron-right-dark.svg"

                    Behavior on rotation {
                        NumberAnimation {
                            duration: animationDuration
                            easing.type: Easing.OutQuad
                        }
                    }
                }
            }
        }

        // Card content
        Column {
            id: column

            Layout.preferredWidth: accordionTitle.width
            Layout.leftMargin: icon.width + VPNTheme.theme.listSpacing * 4
            Layout.rightMargin: VPNTheme.theme.listSpacing

            ColumnLayout {
                id: cardContent

                width: parent.width

                //becomes invisible so that it is no longer calculated in the implicitHeight (or picked up by screen readers)
                visible: opacity > 0

                Behavior on opacity {
                    NumberAnimation {
                        duration: animationDuration * 1.25
                        easing.type: Easing.OutQuad
                    }
                }
            }
        }
    }

    function handleToggleCard() {
        root.expanded = !root.expanded;
    }
}
