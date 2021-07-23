import QtQuick 2.0

Item {
    id: semiMonoLabel

    property string text: ""
    property int fontSize: 15

    implicitHeight: labelRow.height
    implicitWidth: labelRow.width

    Row {
        id: labelRow

        Repeater {
            model: text.split("")

            Item {
                id: digitContainer

                height: fontSize * 1.2
                width: fontSize * 0.75;

                Component.onCompleted: {
                    digitContainer.width = Qt.binding(function getCharWidth() {
                        const narrowCharacters = ":";
                        const defaultWidth = fontSize * 0.7;
                        const narrowWidth = fontSize * 0.4;

                        digitContainer.width = narrowCharacters.includes(modelData) ? narrowWidth : defaultWidth;
                    });
                }

                Text {
                    id: digit
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    color: "#FFFFFF"
                    font.pixelSize: fontSize
                    font.letterSpacing: 0
                    opacity: 0.8
                    text: modelData
                }
            }
        }
    }

}
