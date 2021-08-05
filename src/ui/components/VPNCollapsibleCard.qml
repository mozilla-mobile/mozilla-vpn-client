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

    color: "lightgray"
    implicitHeight: cardWrapper.height
    implicitWidth: parent.width
    radius: Theme.cornerRadius


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

            Layout.bottomMargin: 8
            Layout.topMargin: 8
            Layout.leftMargin: 8
            Layout.rightMargin: 16
            Layout.fillWidth: true

            Rectangle {
               id: icon
               width: 40
               height: 40
            }

            Text {
                id: accordionTitle

//                bottomPadding: 8
                color: "black"
                font.family: Theme.fontBoldFamily
                font.pixelSize: Theme.fontSize
                lineHeightMode: Text.FixedHeight
                lineHeight: Theme.labelLineHeight
                text: root.title
//                text: root.title
//                topPadding: 8
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.Wrap
                Layout.fillWidth: true
                Layout.preferredWidth: cardWrapper.width

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
                   anchors.fill: stateIndicator
                   hoverEnabled: true
                   targetEl: stateIndicator
               }
            }

            Rectangle {
                anchors.fill: parent
                radius: Theme.cornerRadius
                color: "pink"
                z: -1
            }

            MouseArea {
                anchors.fill: accordionHeader
                onClicked: toggleCard()
            }
        }

        // Card content
        Column {
            id: column

            Layout.preferredWidth: accordionTitle.width
            Layout.leftMargin: icon.width + accordionHeader.spacing * 2
            Layout.alignment: Qt.AlignTop

            Text {
                id: cardContent

                anchors {
                    left: parent.left
                    right: parent.right
                }
                bottomPadding: 8
                clip: true
                color: "black"
                font.pixelSize: Theme.fontSizeSmall

                Behavior on height {
                     NumberAnimation {
                         duration: animationDuration
                     }
                }

                text: "Test text content Test text content Test text content Test text content"
                wrapMode: Text.Wrap
                Rectangle {
                    anchors.fill: parent
                    radius: Theme.cornerRadius

                    color: "pink"
                    z: -1
                }
            }
        }

    }

    function toggleCard() {
        root.expanded = !root.expanded;
    }
}
