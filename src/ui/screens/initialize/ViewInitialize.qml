/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import compat 0.1

Item {
    property int safeAreaHeight: window.safeContentHeight
    property string logoSubtitle: MZI18n.ProductDescription

    MZRadialGradient {
        height: Screen.height
        width: Screen.width

        anchors {
            top: parent.top
            topMargin:  -window.safeAreaHeightByDevice()
        }

        gradient: Gradient {
            GradientStop {
                color: MZTheme.theme.onBoardingGradient.start
                position: 0.0
            }
            GradientStop {
                color: MZTheme.theme.onBoardingGradient.middle
                position: 0.2
            }
            GradientStop {
                color: MZTheme.theme.onBoardingGradient.end
                position: 0.5
            }
        }
    }

    MZFlickable {
        id: onboardingPanel

        flickContentHeight: Math.max(window.safeContentHeight / 2 + col.implicitHeight, parent.height)
        height: parent.height
        width: parent.width

        ListModel {
            id: onboardingModel

            ListElement {
                animationSpeed: 1.5
                animationSrc: ":/ui/resources/animations/vpnlogo-drop_animation.json"
                loopAnimation: false
                titleStringId: "MobileOnboardingPanelOneTitle"
                subtitleStringId: "MobileOnboardingPanelOneSubtitle"
                panelId: "mozilla-vpn"
            }
            ListElement {
                animationSpeed: 1
                animationSrc: ":/ui/resources/animations/globe_animation.json"
                loopAnimation: true
                titleStringId: "OnboardingPanelTwoTitle"
                subtitleStringId: "OnboardingPanelTwoSubtitle"
                panelId: "encrypt-your-activity"
            }
            ListElement {
                animationSpeed: 1
                animationSrc: ":/ui/resources/animations/vpnactive_animation.json"
                loopAnimation: true
                titleStringId: "OnboardingPanelThreeTitle"
                subtitleStringId: "OnboardingPanelThreeSubtitle"
                panelId: "protect-your-privacy"
            }
            ListElement {
                animationSpeed: 1
                animationSrc: ":/ui/resources/animations/lock_animation.json"
                loopAnimation: true
                titleStringId: "OnboardingPanelFourTitle"
                subtitleStringId: "OnboardingPanelFourSubtitle"
                panelId: "more-security"
            }
        }

        SwipeView {
            property int _topMargin: safeAreaHeight / 2 - currentPanelValues._animationHeight
            property bool _isFirstSlide: swipeView.currentIndex === 0

            id: swipeView
            objectName: "swipeView"
            anchors.fill: parent
            currentIndex: 0

            Component.onCompleted: {
                contentItem.maximumFlickVelocity = 5 * MZTheme.theme.maxContentWidth;
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

                        MZLottieAnimation {
                            id: panelAnimation

                            property real imageScaleValue: 0.9
                            property real imageOpacityValue: 0.0
                            speed: animationSpeed
                            anchors.fill: undefined
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.top: parent.top
                            anchors.topMargin: swipeView._isFirstSlide ? 0 : swipeView._topMargin
                            height: currentPanelValues._animationHeight + (swipeView._isFirstSlide ? swipeView._topMargin : 0)
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
                                    duration: 0
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
                                        duration: 100
                                        easing.type: Easing.OutQuad
                                    }
                                    PropertyAnimation {
                                        targets: [panelTitle, panelDescription, panelAnimation]
                                        property: "opacity"
                                        from: panelAnimation.imageOpacityValue
                                        to: 1
                                        duration: 100
                                        easing.type: Easing.OutQuad
                                    }
                                }
                            }

                            onVisibleChanged: {
                                if (visible) updatePanel.start();
                            }

                        }

                        Component.onCompleted: {
                            currentPanelValues._panelId = panelId;
                            currentPanelValues._panelTitleText = MZI18n[titleStringId];
                            currentPanelValues._panelDescriptionText = MZI18n[subtitleStringId];
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

        MZHeaderLink {
            id: headerLink
            objectName: "getHelpLink"
            labelText: MZI18n.GetHelpLinkTitle
            isLightTheme: false
            onClicked: {
                Glean.sample.getHelpClickedInitialize.record();
                MZNavigator.requestScreen(VPN.ScreenGetHelp);
            }
            anchors {
                top: parent.top
                topMargin: MZTheme.theme.windowMargin
                right: parent.right
                rightMargin: MZTheme.theme.windowMargin
            }
        }

        QtObject {
            // Stores strings for later injection in updatePanel()
            id: currentPanelValues
            property string _panelId: ""
            property string _panelTitleText: ""
            property string _panelDescriptionText: ""
            property real _animationHeight: Math.min(240, safeAreaHeight * .35)
        }

        ColumnLayout {
            id: col

            anchors {
                fill: parent
                topMargin: onboardingPanel.height / 2
                leftMargin: MZTheme.theme.windowMargin * 1.5
                rightMargin: MZTheme.theme.windowMargin * 1.5
            }

            ColumnLayout {
                id: panelText

                spacing: MZTheme.theme.windowMargin / 2

                MZHeadline {
                    id: panelTitle
                    objectName: "panelTitle"

                    color: MZTheme.colors.white
                    width: undefined
                    Layout.fillWidth: true
                }

                MZSubtitle {
                    id: panelDescription
                    objectName: "panelDescription"
                    color: MZTheme.colors.grey20
                    width: undefined
                    Layout.fillWidth: true
                }
            }


            ColumnLayout {
                id: panelBottomContent
                spacing: MZTheme.theme.windowMargin

                MZVerticalSpacer {
                    // Pushes panelText and PanelBottomContent to top and bottom of
                    // the wrapping ColumnLayout
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                }

                MZInterLabel {
                    
                    text: MZI18n.FreeTrialsStartYourFreeTrial
                    color: MZTheme.colors.white80
                    font.family: MZTheme.theme.fontInterSemiBoldFamily
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignHCenter
                    width: undefined
                    visible: MZFeatureList.get("freeTrial").isSupported
                }
                PageIndicator {
                    id: progressIndicator

                    interactive: true
                    count: swipeView.count
                    currentIndex: swipeView.currentIndex
                    Layout.alignment: Qt.AlignHCenter
                    spacing: MZTheme.theme.windowMargin / 2

                    delegate: Rectangle {
                        id: circle
                        color: index === swipeView.currentIndex ? MZTheme.theme.blue : MZTheme.theme.greyPressed
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

                MZButton {
                    id: signUpButton

                    objectName: "signUpButton"
                    text: MZI18n.MobileOnboardingSignUpBtn
                    Layout.alignment: Qt.AlignHCenter
                    Layout.fillWidth: true
                    width: undefined
                    onClicked: {
                        const platform = Qt.platform.os;
                        if (platform === "android" || platform === "ios") {
                            return onboardingPanel.recordGleanEvtAndStartAuth(objectName)
                        }
                        VPN.authenticate();
                    }
                }

                MZLinkButton {
                    objectName: "alreadyASubscriberLink"
                    labelText: MZI18n.MobileOnboardingAlreadyASubscriber
                    Layout.alignment: Qt.AlignHCenter
                    linkColor: MZTheme.theme.whiteButton
                    width: undefined
                    onClicked: {
                        const platform = Qt.platform.os;
                        if (platform === "android" || platform === "ios") {
                            return onboardingPanel.recordGleanEvtAndStartAuth(objectName)
                        }
                        VPN.authenticate();
                    }
                }

                MZVerticalSpacer {
                    id: spacerBottom
                    Layout.preferredHeight: Math.min(window.height * 0.08, MZTheme.theme.rowHeight)
                }
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

        Component.onCompleted: {
            statusBarModifier.statusBarTextColor = MZTheme.StatusBarTextColorLight;
        }

        MZMobileStatusBarModifier {
            id: statusBarModifier
            statusBarTextColor: MZTheme.StatusBarTextColorLight

        }

        Component.onDestruction: {
            statusBarModifier.resetDefaults();
        }

        function recordGleanEvtAndStartAuth(ctaObjectName) {
            Glean.sample.onboardingCtaClick.record({
                "panel_id": currentPanelValues._panelId,
                "panel_idx": swipeView.currentIndex.toString(),
                "panel_cta": ctaObjectName
            });
            VPN.authenticate();
        }
    }
}
