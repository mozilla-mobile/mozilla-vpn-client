import QtQuick 2.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Mozilla.VPN 1.0
import "../themes/colors.js" as Color

Item {
    id: root

    property alias tabList: tabButtons.model
    property alias stackContent: stack.children

    TabBar {
        id: bar
        width: parent.width
        visible: stack.children.length > 1
        height: stack.children.length === 1 ? 0 : contentHeight
        contentHeight: 56
        background: Rectangle {
            color: Color.grey5
        }

        Repeater {
            id: tabButtons

            delegate: TabButton {
                id: btn
                height: bar.height
                checkable: true

                background: Rectangle {
                    color: Color.grey5
                }

                contentItem: VPNBoldLabel {
                    text: buttonLabel
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    color: btn.checked ? Color.purple70 : btn.hovered ? Color.grey50 : Color.grey40

                    Behavior on color {
                        PropertyAnimation {
                            duration: 100
                        }
                    }
                }
            }
        }
    }

    Rectangle {
        // grey divider
        anchors.bottom: bar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        color: Color.grey10
        height: 1
    }

    Rectangle {
        // active tab indicator
        width: bar.currentItem.width
        height: 2
        color: Color.purple70
        anchors.bottom: bar.bottom
        x: bar.currentItem.x
        Behavior on x {
            PropertyAnimation {
                duration: 100
            }
        }
    }

    StackLayout {
        id: stack
        width: parent.width
        currentIndex: bar.currentIndex
        anchors.top: bar.bottom
        height: root.height - bar.contentHeight
        clip: true

        Component.onCompleted: {
            console.log(bar.children[1].x)
        }

        onCurrentIndexChanged: PropertyAnimation {
                target: stack
                property: "opacity"
                from: 0
                to: 1
                duration: 200
            }

        // pass views to this component using stackContent property

        /*
            stackContent: [
                Item { ...},
                Item { ...}
            ]
       */
    }
}
