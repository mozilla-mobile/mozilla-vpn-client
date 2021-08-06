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

    color: "lightgray"
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

            spacing: 8
            Layout.topMargin: 8
            Layout.leftMargin: 8
            Layout.rightMargin: 16
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

                Rectangle {
                    anchors.fill: parent
                    color: "yellow"
                    z: -1
                }
            }

            Rectangle {
                id: stateIndicator

                color: stateIndicator.state === Theme.uiState.stateHovered ? Color.grey10 : Color.white
                height: 40
                radius: Theme.cornerRadius - 1
                width: 40

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
                        }
                    }
                }

               VPNMouseArea {
                   height: parent.height
                   hoverEnabled: true
                   targetEl: stateIndicator
                   width: parent.width
               }
            }

            Rectangle {
                color: "pink"
                height: parent.height
                radius: Theme.cornerRadius
                width: parent.width
                z: -1
            }

            MouseArea {
                height: parent.height
                onClicked: toggleCard()
                width: parent.width
            }
        }

        // Card content
        Column {
            id: column

            clip: true
            Layout.preferredWidth: accordionTitle.width
            Layout.bottomMargin: 8
            Layout.leftMargin: 16 + icon.width
            Layout.rightMargin: 8

            ColumnLayout {
                id: cardContent

                width: parent.width

                Behavior on height {
                     NumberAnimation {
                         duration: animationDuration
                     }
                }

                Rectangle {
                    color: "pink"
                    height: parent.height
                    radius: Theme.cornerRadius
                    width: parent.width
                    z: -1
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

    function toggleCard() {
        root.expanded = !root.expanded;
    }
}
