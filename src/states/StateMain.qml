import QtQuick 2.0
import QtQuick.Controls 2.5

Item {
    StackView {
        id: stackview
        anchors.fill: parent
        initialItem: "../views/ViewMain.qml"
    }
}
