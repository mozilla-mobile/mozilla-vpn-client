import QtQuick 2.15
import QtQuick.Layouts 1.15

import Mozilla.VPN 1.0

// VPNSegmentedToggle
Rectangle {
    id: root

    property alias model: options.model
    property int selectedIndex: 0
    property QtObject selectedSegment
    property var handleSegmentClick: (() => {});

    onSelectedIndexChanged: {
        selectedSegment = options.itemAt(selectedIndex)
        console.info(selectedSegment.objectName)
    }

    Component.onCompleted: {
        selectedSegment = options.itemAt(selectedIndex)
    }

    implicitHeight: 40
    color: VPNTheme.theme.input.highlight
    radius: 24

    Rectangle {
        id: slider
        anchors {
            top: parent.top
            left: parent.left
            bottom: parent.bottom
            topMargin: 4
            leftMargin: implicitWidth * root.selectedIndex + 4
            rightMargin: 4
            bottomMargin: 4
        }

        implicitWidth: (parent.width - 8) / model.count
        color: VPNTheme.theme.white
        radius: parent.radius

        Behavior on anchors.leftMargin {
            PropertyAnimation {
                duration: 100
            }

        }
    }

    RowLayout {

        anchors {
            fill: parent
            topMargin: 8
            leftMargin: 4
            rightMargin: 4
            bottomMargin: 8
        }

        spacing: 0

        Repeater {
            id: options

            delegate: Item {
                id: segment
                Layout.fillWidth: true
                Layout.fillHeight: true

                objectName: segmentButtonId

                VPNMetropolisLabel {
                    anchors.fill: parent

                    text: VPNl18n[segmentLabelStringId]
                    font.family: VPNTheme.theme.fontBoldFamily
                    color: root.selectedIndex === index ? VPNTheme.colors.purple70 : VPNTheme.colors.grey40

                    Behavior on color {
                        PropertyAnimation {
                            duration: 100
                        }
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        if(root.selectedIndex !== index) {
                            root.selectedIndex = index
                            root.handleSegmentClick(segment)
                        }
                    }
                }
            }
        }
    }
}
