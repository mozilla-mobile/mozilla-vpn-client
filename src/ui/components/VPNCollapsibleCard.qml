import QtQuick 2.15
import QtQuick.Layouts 1.14
import QtQuick.Controls 2.14

import "../themes/themes.js" as Theme
import "../themes/colors.js" as Color

Rectangle {
    id: root

    property int animationDuration: 150
    property bool expanded: false
    property string title: ""
    property string iconSrc: ""
    property alias contentItem: cardContent.data

    color: Color.white
    implicitHeight: cardWrapper.height
    radius: Theme.cornerRadius
    width: parent.width

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

            spacing: Theme.listSpacing * 2
            Layout.topMargin: Theme.listSpacing
            Layout.leftMargin: Theme.listSpacing * 2
            Layout.rightMargin: Theme.listSpacing * 2
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

                color: Color.grey50
                font.family: Theme.fontBoldFamily
                font.pixelSize: Theme.fontSize
                lineHeight: Theme.labelLineHeight
                lineHeightMode: Text.FixedHeight
                text: root.title
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.Wrap
            }

            Rectangle {
                id: stateIndicator

                color: accordionHeader.state === Theme.uiState.stateHovered || accordionHeader.state === Theme.uiState.statePressed
                    ? Color.grey10
                    : Color.white
                height: Theme.rowHeight
                radius: Theme.cornerRadius - 1
                width: Theme.rowHeight

                Behavior on color {
                    ColorAnimation {
                        duration: animationDuration
                    }
                }

                VPNChevron {
                    id: chevron

                    isDark: true
                    anchors {
                        verticalCenter: parent.verticalCenter
                        horizontalCenter: parent.horizontalCenter
                    }

                    Behavior on rotation {
                        NumberAnimation {
                            duration: animationDuration
                            easing.type: Easing.OutQuad
                        }
                    }
                }
            }

            VPNMouseArea {
                anchors.fill: {}
                height: parent.height
                hoverEnabled: true
                onMouseAreaClicked: handleToggleCard
                targetEl: accordionHeader
                width: parent.width
            }
        }

        // Card content
        Column {
            id: column

            clip: true
            Layout.preferredWidth: accordionTitle.width
            Layout.bottomMargin: Theme.listSpacing
            Layout.leftMargin: icon.width + Theme.listSpacing * 4
            Layout.rightMargin: Theme.listSpacing

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

    VPNDropShadow {
        anchors.fill: root
        source: root
        cached: true
        transparentBorder: true
        z: -1
    }

    function handleToggleCard() {
        root.expanded = !root.expanded;
    }
}
