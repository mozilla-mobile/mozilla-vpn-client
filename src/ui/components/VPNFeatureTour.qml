/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14

ColumnLayout {
    id: tour

    signal started()
    signal finished()
    signal close()

    property variant slidesData: [
        {
            title: "What’s new in 2.5?",
            text: "We’ve added a lot of really cool feature in this latest version release! Take the tour if you want a deeper dive into what we’ve added for you!",
            imageSrc: "",
        },
        {
            title: "First slide",
            text: "This is a text about the feature",
            imageSrc: "",
        },
        {
            title: "Second slide",
            text: "Another text about another feature",
            imageSrc: "",
        },
        {
            title: "Third slide",
            text: "Yet another another feature",
            imageSrc: "",
        },
    ]

    states: [
        State {
            name: "default"
            when: swipeView.currentIndex > 0 && swipeView.currentIndex < swipeView.count - 1

            PropertyChanges {
                target: resumeButton
                text: "Next"
            }
        },
        State {
            name: "start"
            when: swipeView.currentIndex === 0

            PropertyChanges {
                target: resumeButton
                text: "Take the tour"
            }
        },
        State {
            name: "end"
            when: swipeView.currentIndex === swipeView.count - 1

            PropertyChanges {
                target: resumeButton
                text: "Done"
            }
        }
    ]

    anchors.horizontalCenter: parent.horizontalCenter
    anchors.verticalCenter: parent.verticalCenter

    SwipeView {
        id: swipeView

        clip: true
        currentIndex: 0
        interactive: true

        height: 100
        Layout.fillWidth: true

        // Slide component
        Component {
            id: slide

            Rectangle {

                height: 100
                width: parent.width

                Text {
                    text: slideData.title

                    Rectangle {
                        color: "limegreen"
                        height: parent.height
                        width: parent.width
                        z: -1
                    }
                }

//                Component.onCompleted: {
//                    console.log("created: ", index);
//                }

//                Component.onDestruction: {
//                    console.log("destroyed: ", index);
//                }

                Rectangle {
                    color: "lightblue"
                    height: parent.height
                    width: parent.width
                    z: -1
                }
            }
        }

        Repeater {
            id: slidesRepeater
            model: slidesData

            Loader {
                id: slideLoader

                property int slideIndex: index
                property variant slideData: modelData

                active: SwipeView.isCurrentItem
                asynchronous: true
                sourceComponent: slide
                visible: slideLoader.status === Loader.Ready
            }
        }
    }

    PageIndicator {
        id: indicator

        count: swipeView.count - 1
        currentIndex: swipeView.currentIndex - 1
        opacity: Math.min(swipeView.currentIndex, 1)

        Layout.alignment: Qt.AlignHCenter

        Rectangle {
            color: "pink"
            height: parent.height
            width: parent.width
            z: -1
        }
    }

    RowLayout {
        Button {
            id: backButton
            enabled: swipeView.currentIndex > 0
            text: "<-"

            onClicked: {
                swipeView.currentIndex -= 1;
            }
        }

        Button {
            id: resumeButton

            onClicked: {
                if (tour.state === "end") {
                    tour.finished();
                    return;
                }

                swipeView.currentIndex += 1;
            }
        }
    }

    Rectangle {
        color: "yellow"
        height: parent.height
        width: parent.width
        z: -1
    }

}
