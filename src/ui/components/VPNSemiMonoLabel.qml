import QtQuick 2.0

Item {
    id: semiMonoLabel

    property string text: ""
    property int fontSize: 15
    property string fontColor: "black"
    property real fontOpacity: 1.0

    implicitHeight: labelRow.height
    implicitWidth: labelRow.width
    opacity: fontOpacity

    Row {
        id: labelRow

        Repeater {
            model: text.split("")

            Item {
                id: digitContainer

                height: fontSize * 1.2

                Component.onCompleted: {
                    digitContainer.width = Qt.binding(function getCharWidth() {
                        const narrowCharacters = [" ", "\t", "\n", ":"];
                        const defaultWidth = fontSize * 0.62;
                        const narrowWidth = fontSize * 0.4;

                        digitContainer.width = narrowCharacters.includes(modelData) ? narrowWidth : defaultWidth;
                    });
                }

                Text {
                    id: digit
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    color: fontColor
                    font.pixelSize: fontSize
                    font.letterSpacing: 0
                    text: modelData
                }
            }
        }
    }

}
