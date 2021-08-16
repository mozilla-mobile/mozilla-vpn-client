/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14

Item {
    id: tour

    height: parent.height
    width: parent.width

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
            title: "Multi-hop VPN",
            text: "Multi-hop VPN will route your traffic thourgh a second server for added protection. You can find this feature on the “Select location” screen.",
            imageSrc: "",
        },
        {
            title: "In-app Support Form",
            text: "The In-app support form will allow you to contact support from within the VPN app. You can find this feature in the “Get help” section.",
            imageSrc: "",
        },
        {
            title: "Custom DNS",
            text: "Custom DNS servers allow for faster speed using local networks, features like ad-blocking and anti-tracking. You can find this feature in “Network settings” section.",
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

            PropertyChanges {
                enabled: true
                opacity: 1.0
                target: backButton
            }
        },
        State {
            name: "start"
            when: swipeView.currentIndex === 0

            PropertyChanges {
                target: resumeButton
                text: "Take the tour"
            }

            PropertyChanges {
                enabled: false
                opacity: 0.2
                target: backButton
            }
        },
        State {
            name: "end"
            when: swipeView.currentIndex === swipeView.count - 1

            PropertyChanges {
                target: resumeButton
                text: "Done"
            }

            PropertyChanges {
                enabled: true
                opacity: 1.0
                target: backButton
            }
        }
    ]

    // Back button
    Button {
        id: backButton
        enabled: swipeView.currentIndex > 0
        text: "<-"
        z: 1

        onClicked: {
            swipeView.currentIndex -= 1;
        }
    }

    ColumnLayout {
        spacing: 0
        width: parent.width

        SwipeView {
            id: swipeView

            clip: true
            currentIndex: 0
            implicitHeight: 300
            interactive: true
            spacing: 0

            Layout.fillWidth: true

            // Slide component
            Component {
                id: slide

                ColumnLayout {
                    height: parent.height
                    width: parent.width

                    Text {
                        text: slideData.title
                        wrapMode: Text.WordWrap

                        Layout.fillWidth: true

                        Rectangle {
                            color: "limegreen"
                            height: parent.height
                            width: parent.width
                            z: -1
                        }
                    }

                    Text {
                        text: slideData.text
                        wrapMode: Text.WordWrap

                        Layout.fillWidth: true

                        Rectangle {
                            color: "limegreen"
                            height: parent.height
                            width: parent.width
                            z: -1
                        }
                    }

                    Component.onCompleted: {
                        console.log("created: ", slideIndex);
                    }

                    Component.onDestruction: {
                        console.log("destroyed: ", slideIndex);
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

        // Dots
        PageIndicator {
            id: indicator

            count: swipeView.count - 1
            currentIndex: swipeView.currentIndex - 1
            opacity: Math.min(swipeView.currentIndex, 1) + 0.2

            Layout.alignment: Qt.AlignHCenter

            Rectangle {
                color: "pink"
                height: parent.height
                width: parent.width
                z: -1
            }
        }

        // Next button
        Button {
            id: resumeButton

            Layout.fillWidth: true

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
        color: "gray"
        height: parent.height
        width: parent.width
        z: -1
    }

}
