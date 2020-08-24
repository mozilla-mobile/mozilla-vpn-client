import QtQuick 2.0
import QtQuick.Controls 2.15
import Mozilla.VPN 1.0

import "../components"

Item {
    VPNMenu {
        id: menu
        title: qsTr("Select location")
    }

    ListView {
        height: parent.height - menu.height
        width: parent.width
        anchors.top: menu.bottom
        clip: true

        model: VPNServerCountryModel

        delegate: ItemDelegate {
            onClicked: cityList.visible = true
            Image {
                source: "../resources/flags/" + code.toUpperCase() + ".png"
                width: 30
                height: 30
            }

            Text {
                id: countryName
                text: name + "("+ code + ")"
            }

            Text {
                anchors.top: countryName.bottom
                id: cityList
                visible: false
                text: cities.join(", ")
            }
        }
    }
}
