import QtQuick 2.5
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import Mozilla.Shared 1.0

//MZStepNavigation is a navigation component(similar to MZSegmentedNavigation and MZTabNavigation) comprised of an MZStepProgressBar in which each step corresponds to a view
//Views reside within a StackView, and as the navigation progresses, new views are pushed on top of the previous views

//Note: Views must be passed in the order that they will appear within the navigation
//Note: Does not consider the navbar

//Usage: pass a list of views to this component using the `views` property
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
    property int progressBarHorizontalMargins: 44

    signal progressBarButtonClicked(previousIndex: int, currentIndex: int)

    function next() {
        if (stepProgressBar.activeIndex + 1 < stepProgressBar.model.count) {
            stackView.push(views[stepProgressBar.activeIndex + 1])
            currentIndex = stepProgressBar.activeIndex + 1
            stepProgressBar.activeIndex++
        }
    }

    function back() {
        if (stepProgressBar.activeIndex - 1 >= 0) {
            currentIndex--
            stepProgressBar.activeIndex--
            stackView.pop()
        }
    }

    function onProgressBarButtonClicked() {
        if(currentIndex === stepProgressBar.activeIndex) return

        //currentIndex holds the index we are navigating from
        //stepProgressBar.activeIndex holds the index we are navigating to
        progressBarButtonClicked(currentIndex, stepProgressBar.activeIndex)

        let previousIndex = currentIndex
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
            stepProgressBarListModel.append({"labelText": views[i].labelText, "iconSource": views[i].iconSource, "objectName": views[i].objectName})
        }

        if(views.length >= 1) stackView.push(views[0])

        //Push views until we are at the index we want (currentIndex)
        for (let i = 1; i <= currentIndex; i++) {
            stackView.push(views[i])
            stepProgressBar.activeIndex++
        }
    }

    ListModel { id: stepProgressBarListModel }

    MZStepProgressBar {
        id: stepProgressBar

        Layout.leftMargin: stepNavigation.progressBarHorizontalMargins
        Layout.rightMargin: stepNavigation.progressBarHorizontalMargins
        Layout.fillWidth: true
        Layout.maximumWidth: 500 //Max size of progess bar for tablets
        Layout.alignment: Qt.AlignHCenter

        model: stepProgressBarListModel

        Connections {
            target: stepProgressBar
            function onButtonClicked(index) { stepNavigation.onProgressBarButtonClicked() }
        }
    }

    MZFlickable {
        id: flickable

        Layout.fillWidth: true

        //Always goes to bottom of the screen - can expose this property if we ever need a custom height
        implicitHeight: window.height - window.safeAreaHeightByDevice() - stepProgressBar.implicitHeight - stepNavigation.anchors.topMargin
        flickContentHeight: stackView.currentItem.implicitHeight + stackView.currentItem.anchors.topMargin + stackView.currentItem.anchors.bottomMargin

        StackView {
            id: stackView
            objectName: "stepNavStackView"

            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right

            implicitHeight: flickable.height
        }
    }

    //Fix for VPN-5663
    Item {
        Layout.fillHeight: true
    }
}
