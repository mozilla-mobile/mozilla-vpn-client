import QtQuick 2.5
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import Mozilla.Shared 1.0

//Usage: pass views to this component using views property
//Note: views must be passed in order that they will appear within the navigation
/*
        views: [
            Item {
                property string labelText: <labelText>
                property string iconSource: <iconSource>

                ...
            },
            Item {
                property string labelText: <labelText>
                property string iconSource: <iconSource>

                ...
            }
        ]
*/

ColumnLayout {
    id: stepNavigation

    property list<Item> views
    property int currentIndex: 0
//    property alias stackView: stackView
//    property alias stepProgressBar: stepProgressBar

    function next() {
        stackView.push(views[stepProgressBar.activeIndex + 1])
        currentIndex = stepProgressBar.activeIndex + 1
        stepProgressBar.activeIndex++
    }

    function back() {
        stackView.pop()
        currentIndex = stepProgressBar.activeIndex - 1
        stepProgressBar.activeIndex--
    }

    function activeIndexChanged() {
        let temp = currentIndex
        for (let i = 0; i < currentIndex - stepProgressBar.activeIndex; i++) {
            temp--
            stackView.pop()
        }
        currentIndex = temp
    }

    spacing: 0

    Component.onCompleted: {
        //Fill the MZStepProgressBar model with labels and icons
        for (let i = 0; i < views.length; i++) {
            stepProgressBarListModel.append({"labelText": views[i].labelText, "iconSource": views[i].iconSource})
        }
    }

    ListModel { id: stepProgressBarListModel }

    MZStepProgressBar {
        id: stepProgressBar

        //May need to eventually expose these margins so they can be modified externally
        Layout.topMargin: 24
        Layout.leftMargin: 44
        Layout.rightMargin: 44
        Layout.fillWidth: true
        Layout.maximumWidth: 500 //Max size of progess bar for tablets

        onActiveIndexChanged: stepNavigation.activeIndexChanged()

        model: stepProgressBarListModel
    }

    MZFlickable {
        id: flickable

        Layout.fillWidth: true
        Layout.fillHeight: true
        flickContentHeight: stackView.currentItem.implicitHeight + stackView.currentItem.anchors.topMargin + stackView.currentItem.anchors.bottomMargin

        StackView {
            id: stackView

            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right

            implicitWidth: flickable.width
            implicitHeight: flickable.height

            Component.onCompleted: {
                push(stepNavigation.views[0])
            }
        }
    }

}
