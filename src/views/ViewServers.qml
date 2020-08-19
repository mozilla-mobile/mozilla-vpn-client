import QtQuick 2.0
import QtQuick.Controls 2.15
import Mozilla.VPN 1.0

Item {
    Text {
        id: backLink
        color: "#0a68e3"
        text: qsTr("back")
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.topMargin: 16
        anchors.rightMargin: 16
        MouseArea {
            onClicked: stackview.pop()
            cursorShape: Qt.PointingHandCursor
            anchors.fill: parent
            hoverEnabled: true
        }
    }

    ListView {
        height: parent.height - 20
        width: parent.width
        anchors.top: backLink.bottom

        model: VPN.serverCountryModel

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
