import QtQuick 2.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.11
import Mozilla.VPN 1.0

import "../components"

Item {
    VPNMenu {
        id: menu
        title: qsTr("Get Help")
    }

    ListView {
        height: parent.height - menu.height
        width: parent.width
        anchors.top: menu.bottom

        model: ListModel {
            ListElement {
                title: qsTr("Contact us")
                url: "contact_us"
            }

            ListElement {
                title: qsTr("Help & Support")
                url: "help_support"
            }
        }

        delegate: ItemDelegate {
            id: device
            width: parent.width
            checkable: false

            contentItem: RowLayout {
                spacing: 0

                Label {
                    text: title
                }

                Item {
                    Layout.fillWidth: true
                }

                Image {
                    height: 16
                    width: 16
                    Layout.alignment: Qt.AlignTop
                    source: "../resources/delete.svg" // TODO

                    MouseArea {
                        anchors.fill: parent
                        onClicked: VPN.openLink(url)
                    }
                }
            }
        }
    }
}
