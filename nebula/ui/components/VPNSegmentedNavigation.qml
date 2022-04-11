import QtQuick 2.15
import QtQuick.Layouts 1.15

import Mozilla.VPN 1.0

Item {
    id: root

    property alias segmentedToggleButtonsModel: segmentedToggle.model
    property alias stackContent: stack.children
    property alias selectedIndex: segmentedToggle.selectedIndex
    property alias selectedSegment: segmentedToggle.selectedSegment
    property var handleSegmentClick: (()=> {});

    function setSelectedIndex(idx) {
        segmentedToggle.selectedIndex = idx;
    }

    ColumnLayout {
        anchors.fill: parent

        spacing: 0

        VPNSegmentedToggle {
            id: segmentedToggle

            Layout.leftMargin: VPNTheme.theme.windowMargin
            Layout.rightMargin: VPNTheme.theme.windowMargin
            Layout.fillWidth: true

            visible: stack.children.length > 1

            handleSegmentClick: root.handleSegmentClick
        }

        Rectangle {
            Layout.topMargin: VPNTheme.theme.vSpacingSmall
            Layout.fillWidth: true

            Layout.preferredHeight: 1
            color: VPNTheme.colors.grey10
            visible: stack.children.length > 1
        }

        StackLayout {
            id: stack

            Layout.fillHeight: true
            Layout.fillWidth: true
            currentIndex: segmentedToggle.selectedIndex

            PropertyAnimation {
                id: fadeIn
                target: stack
                property: "opacity"
                from: 0
                to: 1
                duration: 200
            }

            onCurrentIndexChanged: {
                fadeIn.start();
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
}
