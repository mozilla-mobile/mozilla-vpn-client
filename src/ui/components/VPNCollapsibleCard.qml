import QtQuick 2.0
import QtQuick.Layouts 1.14
import QtQuick.Controls 2.14

Rectangle {
    id: root

    property bool expanded: false
    property string title: ""
    property real cardContentHeight: 50

    color: "pink"
    implicitHeight: column.height
    implicitWidth: parent.width
    radius: 5

    states: [
        State {
            name: "expanded"
            when: root.expanded

            PropertyChanges {
                target: stateIndicator
                color: "green"
            }

            PropertyChanges {
                target: cardContent
                height: cardContent.implicitHeight
            }
        },
        State {
            name: "collapsed"
            when: !root.expanded

            PropertyChanges {
                target: stateIndicator
                color: "red"
            }

            PropertyChanges {
                target: cardContent
                height: 0
            }
        }
    ]

    Column {
        id: column

        // Card header
        Item {
            id: accordionHeader

            implicitHeight: accordionTitle.height
            implicitWidth: root.width

            Text {
                id: accordionTitle

                color: "black"
                text: root.title
                width: root.width
                wrapMode: Text.Wrap
            }

            Rectangle {
               id: stateIndicator

               anchors {
                   right: accordionHeader.right
                   verticalCenter: accordionHeader.verticalCenter
               }
               width: 16
               height: 16
               radius: width / 2
            }

            MouseArea {
                anchors.fill: parent
                onClicked: toggleCard()
            }
        }

        // Card content
        Text {
            id: cardContent

            color: "black"
            clip: true
            width: column.width

            Behavior on height {
                 NumberAnimation {
                     duration: 200
                 }
            }

            text: "Test text content"
        }
    }

    function toggleCard() {
        root.expanded = !root.expanded;
    }
}
