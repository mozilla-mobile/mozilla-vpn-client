/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
import "../themes/themes.js" as Theme

Item {
    id: tour

    implicitHeight: content.implicitHeight
    width: parent.width

    signal started()
    signal finished()
    signal close()

    property variant slidesData: [
        {
            title: "What’s new in 2.5?",
            text: "We’ve added a lot of really cool feature in this latest version release! Take the tour if you want a deeper dive into what we’ve added for you!",
            imageSrc: "../resources/quick-access.svg",
        },
        {
            title: "Multi-hop VPN",
            text: "Multi-hop VPN will route your traffic thourgh a second server for added protection. You can find this feature on the “Select location” screen.",
            imageSrc: "../resources/shield-off.svg",
        },
        {
            title: "In-app Support Form",
            text: "The In-app support form will allow you to contact support from within the VPN app. You can find this feature in the “Get help” section.",
            imageSrc: "../resources/globe.svg",
        },
        {
            title: "Custom DNS",
            text: "Custom DNS servers allow for faster speed using local networks, features like ad-blocking and anti-tracking. You can find this feature in “Network settings” section.",
            imageSrc: "../resources/shield-on.svg",
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

            PropertyChanges {
                target: indicator
                opacity: 1.0
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
                opacity: 0.5
                target: backButton
            }

            PropertyChanges {
                target: indicator
                opacity: 0
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

            PropertyChanges {
                target: indicator
                opacity: 1.0
            }
        }
    ]

    // Back button
    VPNIconButton {
        id: backButton

        accessibleName: "back button"
        z: 1
        onClicked: {
            swipeView.currentIndex -= 1;
        }

        Image {
            id: backImage

            anchors.centerIn: backButton
            fillMode: Image.PreserveAspectFit
            source: "../resources/back-dark.svg"
            sourceSize.width: Theme.iconSize
        }
    }

    ColumnLayout {
        id: content

        spacing: 0
//        height: parent.height
        width: parent.width

        SwipeView {
            id: swipeView

            clip: true
            currentIndex: 0
            interactive: true

            Layout.fillHeight: true
            Layout.fillWidth: true

            // Slide component
            Component {
                id: slide

                ColumnLayout {
                    id: content
                    spacing: Theme.vSpacingSmall

                    Image {
                        source: slideData.imageSrc
                        sourceSize.height: 120
                        sourceSize.width: 120

                        Layout.alignment: Qt.AlignHCenter
                    }

                    VPNMetropolisLabel {
                        id: popupTitle

                        color: Theme.fontColorDark
                        horizontalAlignment: Text.AlignHCenter
                        font.pixelSize: Theme.fontSizeLarge
                        text: slideData.title

                        Layout.fillWidth: true

                        Rectangle {
                            color: "green"
                            height: parent.height
                            width: parent.width
                            z: -1
                        }
                    }

                    VPNTextBlock {
                        horizontalAlignment: Text.AlignHCenter
                        text: slideData.text
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
            interactive: false
            spacing: Theme.windowMargin / 2
            delegate: Rectangle {
                id: circle

                color: index === indicator.currentIndex ? Theme.blue : Theme.greyPressed
                height: 6
                width: 6
                radius: 6

                Behavior on color {
                    ColorAnimation {
                        duration: 400
                    }
                }
            }

            Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom

            Behavior on opacity {
                NumberAnimation {
                    duration: 100
                }
            }
        }


        // Next button 
        VPNButton {
            id: resumeButton

            radius: Theme.cornerRadius

            Layout.fillWidth: true
            Layout.alignment: Qt.AlignBottom

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
        color: "lightgray"
        height: parent.height
        width: parent.width
        z: -1
    }

}
