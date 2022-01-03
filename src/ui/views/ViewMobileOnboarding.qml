/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1

VPNFlickable {
    id: onboardingPanel
    property real panelHeight: window.safeContentHeight
    flickContentHeight: window.safeContentHeight / 2 + col.implicitHeight

    ListModel {
        id: onboardingModel

        ListElement {
            imageSrc: "qrc:/ui/resources/onboarding/mobile/vpn-off-lightMode.svg"
            headline:qsTrId("vpn.main.productName")
            subtitle: qsTrId("vpn.main.productDescription")
        }
        ListElement {
            imageSrc: "qrc:/ui/resources/onboarding/mobile/vpn-security-lightMode.svg"
            headline: qsTrId("vpn.onboarding.headline.1")
            subtitle: qsTrId("vpn.onboarding.subtitle.1")
        }
        ListElement {
            imageSrc: "qrc:/ui/resources/onboarding/mobile/vpn-on-lightMode.svg"
            headline: qsTrId("vpn.onboarding.headline.3")
            subtitle: qsTrId("vpn.onboarding.subtitle.3")
        }
        ListElement {
            imageSrc: "qrc:/ui/resources/onboarding/mobile/vpn-globe-flags-lightMode.svg"
            headline: qsTrId("vpn.onboarding.headline.2")
            subtitle: qsTrId("vpn.onboarding.subtitle.2")
        }
    }

    SwipeView {
        id: swipeView
        objectName: "swipeView"
        anchors.fill: parent
        currentIndex: 0

        Component.onCompleted:{
            contentItem.maximumFlickVelocity = 5 * VPNTheme.theme.maxContentWidth;
            contentItem.snapMode = ListView.SnapOneItem;
        }

        Repeater {
            id: repeater
            model: onboardingModel

            Loader {
                id: loader
                active: SwipeView.isCurrentItem
                sourceComponent: SwipeDelegate {
                    background: Rectangle {
                        color: "transparent"
                    }

                    Image {
                        id: panelImg

                        property real imageScaleValue: 0.9
                        property real imageOpacityValue: 0.0

                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.top: parent.top
                        anchors.topMargin: onboardingPanel.panelHeight / 2 - currentPanelValues._imageHeight
                        antialiasing: true
                        fillMode: Image.PreserveAspectFit
                        opacity: panelImg.imageOpacityValue
                        source: imageSrc
                        sourceSize.height: currentPanelValues._imageHeight
                        transform: Scale {
                            origin.x: panelImg.width / 2
                            origin.y: panelImg.height / 2
                            xScale: panelImg.imageScaleValue
                            yScale: panelImg.imageScaleValue
                        }

                        SequentialAnimation {
                            id: updatePanel

                            function updateStrings() {
                                panelTitle.text = currentPanelValues._panelTitleText;
                                panelDescription.text = currentPanelValues._panelDescriptionText;
                            }
                            PropertyAnimation {
                                targets: [panelTitle, panelDescription]
                                property: "opacity"
                                from: 1
                                to: panelImg.imageOpacityValue
                                duration: 100
                            }
                            PauseAnimation {
                                duration: 150
                            }
                            ScriptAction {
                                script: updatePanel.updateStrings()
                            }
                            ParallelAnimation {
                                NumberAnimation {
                                    target: panelImg
                                    property: "imageScaleValue"
                                    from: panelImg.imageScaleValue
                                    to: 1
                                    duration: 250
                                    easing.type: Easing.OutQuad
                                }
                                PropertyAnimation {
                                    targets: [panelTitle, panelDescription, panelImg]
                                    property: "opacity"
                                    from: panelImg.imageOpacityValue
                                    to: 1
                                    duration: 250
                                    easing.type: Easing.OutQuad
                                }
                            }
                        }
                    }
                    onClicked: {
                        if (swipeView.currentIndex < onboardingModel.count - 1) {
                            swipeView.currentIndex += 1;
                        } else {
                            swipeView.currentIndex = 0;
                        }
                    }
                    Component.onCompleted: {
                        currentPanelValues._panelTitleText = headline;
                        currentPanelValues._panelDescriptionText = subtitle;
                        updatePanel.start();
                    }
                }
            }
        }
    }

    VPNHeaderLink {
        id: headerLink
        objectName: "getHelpLink"
        labelText: qsTrId("vpn.main.getHelp2")
        onClicked: stackview.push("qrc:/ui/views/ViewGetHelp.qml", StackView.Immediate)
    }

    QtObject {
        // Stores strings for later injection in updatePanel()
        id: currentPanelValues
        property string _panelTitleText: ""
        property string _panelDescriptionText: ""
        property real _imageHeight: Math.min(240, panelHeight * .35)
    }

    ColumnLayout {
        id: col
        anchors.fill: parent
        anchors.topMargin: onboardingPanel.panelHeight / 2
        anchors.rightMargin: VPNTheme.theme.windowMargin * 2
        anchors.leftMargin: VPNTheme.theme.windowMargin * 2

        Column {
            id: panelText
            Layout.preferredWidth: col.width
            spacing: VPNTheme.theme.windowMargin / 2

            VPNHeadline {
                id: panelTitle
                objectName: "panelTitle"
                width: parent.width
            }

            VPNSubtitle {
                id: panelDescription
                objectName: "panelDescription"
                width: parent.width
            }
        }

        VPNVerticalSpacer  {
            // Pushes panelText and PanelBottomContent to top and bottom of
            // the wrapping ColumnLayout
            Layout.fillHeight: true
            Layout.minimumHeight: VPNTheme.theme.windowMargin
        }

        Column {
            id: panelBottomContent
            Layout.preferredWidth: parent.width
            spacing: VPNTheme.theme.windowMargin

            PageIndicator {
                id: progressIndicator

                interactive: true
                count: swipeView.count
                currentIndex: swipeView.currentIndex
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: VPNTheme.theme.windowMargin / 2

                delegate: Rectangle {
                    id: circle
                    color: index === swipeView.currentIndex ? VPNTheme.theme.blue : VPNTheme.theme.greyPressed
                    height: 6
                    width: 6
                    radius: 6
                    Behavior on color {
                        ColorAnimation {
                            duration: 200
                        }
                    }
                }
            }

            VPNButton {
                id: signUpButton
                objectName: "signUpButton"
                anchors.horizontalCenter: parent.horizontalCenter
                text: VPNl18n.MobileOnboardingSignUpBtn
                width: Math.min(parent.width, VPNTheme.theme.maxHorizontalContentWidth)

                // TODO: Add Glean event
                onClicked: VPN.getStarted()
            }

            VPNLinkButton {
                objectName: "alreadyASubscriberLink"
                labelText: VPNl18n.MobileOnboardingAlreadyASubscriber
                anchors.horizontalCenter: parent.horizontalCenter
                height: VPNTheme.theme.rowHeight

                // TODO: Add Glean event
                onClicked: VPN.getStarted()
            }
        }

        VPNVerticalSpacer {
            Layout.preferredHeight: Math.min(window.height * 0.08, VPNTheme.theme.rowHeight)
        }
    }
}
