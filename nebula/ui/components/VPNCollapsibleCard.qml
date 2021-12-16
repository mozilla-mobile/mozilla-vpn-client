/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Layouts 1.14
import QtQuick.Controls 2.14

import Mozilla.VPN 1.0

Rectangle {
    id: root

    property int animationDuration: 150
    property bool expanded: false
    property string title: ""
    property string iconSrc: ""
    property alias contentItem: cardContent.data

    color: VPNTheme.colors.white
    implicitHeight: cardWrapper.height
    radius: VPNTheme.theme.cornerRadius
    width: parent.width

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
                height: cardContent.implicitHeight
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
                height: 0
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

        spacing: 0
        width: root.width

        // Card header
        RowLayout {
            id: accordionHeader

            spacing: VPNTheme.theme.listSpacing * 2
            Layout.topMargin: VPNTheme.theme.listSpacing
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
            }

            VPNIconButton {
                id: stateIndicator

                onClicked: {
                    handleToggleCard();
                }
                radius: VPNTheme.theme.cornerRadius - 1
                Layout.preferredHeight: VPNTheme.theme.rowHeight
                Layout.preferredWidth: VPNTheme.theme.rowHeight

                // TODO - Use accesibleName string
                accessibleName: ""

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

            clip: true
            Layout.preferredWidth: accordionTitle.width
            Layout.bottomMargin: VPNTheme.theme.listSpacing
            Layout.leftMargin: icon.width + VPNTheme.theme.listSpacing * 4
            Layout.rightMargin: VPNTheme.theme.listSpacing

            ColumnLayout {
                id: cardContent

                width: parent.width

                Behavior on height {
                     NumberAnimation {
                         duration: animationDuration
                         easing.type: Easing.OutQuad
                     }
                }

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
