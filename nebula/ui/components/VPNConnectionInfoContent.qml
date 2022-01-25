import QtQuick 2.5
import QtQuick.Layouts 1.14
import QtQuick.Controls 2.14

import Mozilla.VPN 1.0

Flickable {
    id: root

    height: parent.height
    contentHeight: Math.max(content.height, height)
    width: parent.width

    ColumnLayout {
        id: content

        spacing: VPNTheme.theme.vSpacingSmall
        width: parent.width

        // IP Adresses
        RowLayout {
            width: parent.width

            ColumnLayout {
                Text {
                    color: "white"
                    text: "IP: 103.231.88.10"
                }
                Text {
                    color: "white"
                    text: "IPv6: 2001:ac8:40:b9::a09e"
                }
            }
        }

        Rectangle {
            color: "black"
            height: 2
            Layout.fillWidth: true
        }

        // Lottie animation
        Rectangle {
            color: "black"

            Layout.alignment: Qt.AlignHCenter
            Layout.preferredHeight: 100
            Layout.preferredWidth: 100
            Layout.minimumHeight: 50
            Layout.minimumWidth: 50
        }

        Rectangle {
            color: "black"
            height: 2
            Layout.fillWidth: true
        }

        // Bullet list
        RowLayout {

            ColumnLayout {
                Text {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap

                    color: "white"
                    text: "At your current speed, here's what your device is optimized for:"
                }
                Text {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    color: "white"
                    text: "Streaming in 4K"
                }
                Text {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    color: "white"
                    text: "High-speed downloads"
                }
                Text {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    color: "white"
                    text: "Online gaming"
                }
            }

        }

        Rectangle {
            color: "black"
            height: 2
            Layout.fillWidth: true
        }

        // Detailed info section
        RowLayout {
            width: parent.width

            Text {
                color: "white"
                text: "Detailed info section"
            }
        }

        GridLayout {
            width: parent.width
            columns: 2

            Text {
                color: "white"
                text: "One"
            }
            Text {
                color: "white"
                text: "Two"
            }
            Text {
                color: "white"
                text: "Three"
            }
            Text {
                color: "white"
                text: "Four"
            }
        }

        Rectangle {
            color: "gray"

            anchors.fill: parent
            border.color: "red"
            border.width: 2
            z: -1
        }

    }

}