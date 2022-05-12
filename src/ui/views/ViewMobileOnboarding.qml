
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import compat 0.1

import org.mozilla.Glean 0.30
import telemetry 0.30

VPNFlickable {
    id: onboardingPanel

    property real panelHeight: window.safeContentHeight
    property bool shouldRestoreSlide: false

    flickContentHeight: window.safeContentHeight / 2 + col.implicitHeight
    height: parent.height
    interactive: flickContentHeight > height

    VPNMobileStatusBarModifier {
        statusBarTextColor: VPNTheme.StatusBarTextColorLight
    }

    ListModel {
        id: onboardingModel

        ListElement {
            animationSrc: ":/nebula/resources/animations/vpnlogo-drop_animation.json"
            loopAnimation: false
            titleStringId: "MobileOnboardingPanelOneTitle"
            subtitleStringId: "MobileOnboardingPanelOneSubtitle"
            panelId: "mozilla-vpn"
        }
        ListElement {
            animationSrc: ":/nebula/resources/animations/lock_animation.json"
            loopAnimation: true
            titleStringId: "MobileOnboardingPanelTwoTitle"
            subtitleStringId: "MobileOnboardingPanelTwoSubtitle"
            panelId: "encrypt-your-activity"
        }
        ListElement {
            animationSrc: ":/nebula/resources/animations/globe_animation.json"
            loopAnimation: true
            titleStringId: "MobileOnboardingPanelThreeTitle"
            subtitleStringId: "MobileOnboardingPanelThreeSubtitle"
            panelId: "protect-your-privacy"
        }
        ListElement {
            animationSrc: ":/nebula/resources/animations/vpnactive_animation.json"
            loopAnimation: true
            titleStringId: "MobileOnboardingPanelFourTitle"
            subtitleStringId: "MobileOnboardingPanelFourSubtitle"
            panelId: "more-security"
        }
    }

    StackView.onActivating: if (shouldRestoreSlide) {
        shouldRestoreSlide = false;
        goToPreviousSlide();
    }

    SwipeView {
        id: swipeView
        objectName: "swipeView"
        anchors.fill: parent
        currentIndex: 0

        Component.onCompleted: {
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
                    background: Item {}

                    VPNLottieAnimation {
                        id: panelAnimation

                        property real imageScaleValue: 0.9
                        property real imageOpacityValue: 0.0
                        property int _topMargin: onboardingPanel.panelHeight / 2 - currentPanelValues._animationHeight
                        property bool _isFirstSlide: swipeView.currentIndex === 0

                        anchors.fill: undefined
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.top: parent.top
                        anchors.topMargin: panelAnimation._isFirstSlide ? 0 : panelAnimation._topMargin
                        height: currentPanelValues._animationHeight + (panelAnimation._isFirstSlide ? panelAnimation._topMargin : 0)
                        loop: loopAnimation
                        opacity: panelAnimation.imageOpacityValue
                        source: animationSrc
                        transform: Scale {
                            origin.x: panelAnimation.width / 2
                            origin.y: panelAnimation.height / 2
                            xScale: panelAnimation.imageScaleValue
                            yScale: panelAnimation.imageScaleValue
                        }
                        width: parent.width

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
                                to: panelAnimation.imageOpacityValue
                                duration: 100
                            }
                            PauseAnimation {
                                duration: 150
                            }
                            ScriptAction {
                                script: {
                                    updatePanel.updateStrings();
                                    panelAnimation.play();
                                }
                            }
                            ParallelAnimation {
                                NumberAnimation {
                                    target: panelAnimation
                                    property: "imageScaleValue"
                                    from: panelAnimation.imageScaleValue
                                    to: 1
                                    duration: 250
                                    easing.type: Easing.OutQuad
                                }
                                PropertyAnimation {
                                    targets: [panelTitle, panelDescription, panelAnimation]
                                    property: "opacity"
                                    from: panelAnimation.imageOpacityValue
                                    to: 1
                                    duration: 250
                                    easing.type: Easing.OutQuad
                                }
                            }
                        }

                    }

                    Component.onCompleted: {
                        currentPanelValues._panelId = panelId;
                        currentPanelValues._panelTitleText = VPNl18n[titleStringId];
                        currentPanelValues._panelDescriptionText = VPNl18n[subtitleStringId];
                        updatePanel.start();
                    }

                    MouseArea {
                        property int previousMouseX: 0
                        property int swipeDirection: 0

                        anchors.fill: parent

                        onPressed: {
                            previousMouseX = parseInt(mouseX);
                            swipeDirection = 0;
                        }

                        onReleased: {
                            if (swipeDirection < 0) {
                                onboardingPanel.goToPreviousSlide();
                            } else if (swipeDirection > 0) {
                                onboardingPanel.goToNextSlide();
                            } else {
                                onboardingPanel.goToNextSlide();
                            }
                        }

                        onPositionChanged: {
                            const parsedMouseX = parseInt(mouseX);
                            if (previousMouseX > parsedMouseX) {
                                swipeDirection = 1;
                            } else if (previousMouseX < parsedMouseX) {
                                swipeDirection = -1;
                            } else {
                                swipeDirection = 0;
                            }
                            previousMouseX = parsedMouseX;
                        }
                    }

                }
            }
        }
    }

    VPNHeaderLink {
        id: headerLink
        objectName: "getHelpLink"
        labelText: qsTrId("vpn.main.getHelp2")
        isLightTheme: false
        onClicked: {
            if (!shouldRestoreSlide) {
                shouldRestoreSlide = true;
                goToNextSlide();
            }
            getHelpViewNeeded();
        }

        anchors.topMargin: VPNTheme.theme.listSpacing
    }

    QtObject {
        // Stores strings for later injection in updatePanel()
        id: currentPanelValues
        property string _panelId: ""
        property string _panelTitleText: ""
        property string _panelDescriptionText: ""
        property real _animationHeight: Math.min(240, panelHeight * .35)
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
                color: VPNTheme.colors.white
                width: parent.width
            }

            VPNSubtitle {
                id: panelDescription
                objectName: "panelDescription"
                color: VPNTheme.colors.grey20
                width: parent.width
            }
        }

        VPNVerticalSpacer {
            // Pushes panelText and PanelBottomContent to top and bottom of
            // the wrapping ColumnLayout
            Layout.fillHeight: true
            Layout.minimumHeight: VPNTheme.theme.windowMargin
        }

        Column {
            id: panelBottomContent
            Layout.preferredWidth: parent.width
            spacing: VPNTheme.theme.windowMargin

            VPNInterLabel {
                anchors.horizontalCenter: parent.horizontalCenter
                text: VPNl18n.FreeTrialsStartYourFreeTrial
                color: VPNTheme.colors.white80
                font.family: VPNTheme.theme.fontInterSemiBoldFamily
                width: parent.width
                visible: VPNFeatureList.get("freeTrial").isSupported
            }

            VPNVerticalSpacer {
                height: 1
            }

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
                onClicked: onboardingPanel.recordGleanEvtAndStartAuth(objectName)
            }

            VPNLinkButton {
                objectName: "alreadyASubscriberLink"
                labelText: VPNl18n.MobileOnboardingAlreadyASubscriber
                anchors.horizontalCenter: parent.horizontalCenter
                height: VPNTheme.theme.rowHeight
                linkColor: VPNTheme.theme.whiteButton
                onClicked: onboardingPanel.recordGleanEvtAndStartAuth(objectName)
            }
        }

        VPNVerticalSpacer {
            id: spacerBottom
            Layout.preferredHeight: Math.min(window.height * 0.08, VPNTheme.theme.rowHeight)
        }
    }

    function goToNextSlide() {
        if (swipeView.contentItem.currentIndex < onboardingModel.count - 1) {
            swipeView.contentItem.currentIndex += 1;
        } else {
            swipeView.contentItem.currentIndex = 0;
        }
    }

    function goToPreviousSlide() {
        if (swipeView.contentItem.currentIndex > 0) {
            swipeView.contentItem.currentIndex -= 1;
        } else {
            swipeView.contentItem.currentIndex = onboardingModel.count - 1;
        }
    }

    VPNRadialGradient {
        id: mobileOnboardingBackground
        height: Screen.height
        width: Screen.width
        gradient: Gradient {
            GradientStop {
                color: VPNTheme.theme.onBoardingGradient.start
                position: 0.0
            }
            GradientStop {
                color: VPNTheme.theme.onBoardingGradient.middle
                position: 0.2
            }
            GradientStop {
                color: VPNTheme.theme.onBoardingGradient.end
                position: 0.5
            }
        }

        z: -1

        // Hide background if 'Get help' is clicked, or another view is opened from system tray
        visible: mainStackView.depth === 1
    }

    Component.onCompleted: fullScreenMobileBackground.data.push(mobileOnboardingBackground)
    Component.onDestruction: fullScreenMobileBackground.data.pop()

    function recordGleanEvtAndStartAuth(ctaObjectName) {
        Sample.onboardingCtaClick.record({
                                              "panel_id": currentPanelValues._panelId,
                                              "panel_idx": swipeView.currentIndex.toString(),
                                              "panel_cta": ctaObjectName
                                          });
        VPN.getStarted();
    }
}
