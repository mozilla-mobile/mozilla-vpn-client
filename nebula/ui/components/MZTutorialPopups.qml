/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import compat 0.1

Item {
    id: root
    objectName: "tutorialUiRoot"

    /*
        TODOs

        - Handle banner alerts
        - Handle conflicts with update required

        - Should tutorials timeout?
        - Better UI in landscape mode
        - Respond to targetElement y position changes
        - Animation polish on tooltip open

        - Update event filter
            - Restrict wheel events
            - Consider restricting key commands to allowed items

        - Second tutorial nits:
            - Toggle checkbox status with space bar does not trigger next step
            - System preferences is not immediately focused, it's possible to tab to the back arrow and get in a weird state
    */


    // targetElement is set to `parent` here to get around `Cannot call method ... of undefined` warnings
    // and is reset before the tutorial is opened in onTooltipNeeded()
    property var targetElement: parent

    //Temporarily holds the next tooltips text (fixes bug on android where text was changing before the tooltip moved)
    property var tempTooltipText

    anchors.fill: parent
    visible: tutorialTooltip.visible || tutorialPopup.opened

    onTargetElementChanged: {
        if(targetElement && MZTutorial.playing) {
            tutorialTooltip.close()
            pauseTimer.start()
        }
    }

    //Adds a delay between tooltip popups in tutorials
    Timer {
        id: pauseTimer
        interval: 300
        onTriggered: {
            tutorialTooltip.repositionTooltip()
            notch.repositionNotch()
            tutorialTooltip.tooltipText = root.tempTooltipText;
        }
    }

    Popup {
        property alias tooltipText: tooltipText.text
        property int notchHeight: MZTheme.theme.windowMargin
        property bool tooltipPositionedAboveTargetElement: false

        id: tutorialTooltip

        closePolicy: Popup.NoAutoClose
        onClipChanged: MZTutorial.stop();
        verticalPadding: MZTheme.theme.windowMargin
        horizontalPadding: MZTheme.theme.windowMargin

        function repositionTooltip() {
            pushFocusToTargetElement()
            if (targetElement) {
               const windowHeight = window.height;
               const targetElementDistanceFromTop = targetElement.mapToItem(window.contentItem, 0, 0).y

               if (targetElementDistanceFromTop + targetElement.height + tutorialTooltip.implicitHeight > windowHeight) {
                   tooltipPositionedAboveTargetElement = true;
                   y = targetElementDistanceFromTop - targetElement.height - notchHeight * 2.5;
                   tutorialTooltip.open()
                   tutorialTooltip.visible = true
                   return
               }

               tooltipPositionedAboveTargetElement = false;
               y = targetElementDistanceFromTop + targetElement.height + notchHeight;
               tutorialTooltip.open()
               tutorialTooltip.visible = true
               return
           }
        }

        width: root.width - MZTheme.theme.windowMargin * 2
        visible: false
        x: MZTheme.theme.windowMargin

        background: Rectangle {
            id: backgroundRect
            color:  MZTheme.colors.white80
            radius: 4

            MZDropShadow {
                id: notchShadow
                source: notchOutline
                anchors.fill: notchOutline
                transparentBorder: shadow.transparentBorder
                radius: shadow.radius
                color: shadow.color
                cached: true
                rotation: notch.rotation
            }

            MZDropShadow {
                id: shadow
                source: tooltipOutline
                anchors.fill: tooltipOutline
                transparentBorder: true
                radius: 7.5
                color: MZTheme.colors.grey60
                cached: true
            }

            Rectangle {
                id: notchOutline
                height: tutorialTooltip.notchHeight + 2
                width: tutorialTooltip.notchHeight + 2
                radius: notch.radius
                color: notch.color
                rotation: notch.rotation
                anchors.centerIn: notch
                border.width: tooltipOutline.width
                border.color: MZTheme.colors.white80
            }

            Rectangle {
                id: tooltipOutline
                color: MZTheme.colors.primary
                radius: parent.radius
                anchors.fill: parent
                border.width: 1
                border.color: MZTheme.colors.white80
            }

            Rectangle {
                property int tooltipOffset: tutorialTooltip.notchHeight / 4
                id: notch
                height: tutorialTooltip.notchHeight
                width: tutorialTooltip.notchHeight
                radius: 2
                color: tooltipOutline.color
                y: tutorialTooltip.tooltipPositionedAboveTargetElement ? tutorialTooltip.height - (tutorialTooltip.notchHeight - tooltipOffset) : tooltipOffset * -1
                rotation: 45
                anchors.left: parent.left

                function repositionNotch() {
                    if (targetElement && typeof(targetElement) !== undefined) {
                        anchors.leftMargin = (targetElement.mapToItem(window.contentItem, 0, 0).x + (targetElement.width / 2)) - tutorialTooltip.x - tutorialTooltip.notchHeight/2
                    }
                }
            }
        }

        contentItem: RowLayout {
           id: content
           spacing: MZTheme.theme.windowMargin / 2

           MZTextBlock {
                id: tooltipText
                text: ""
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                verticalAlignment: Text.AlignVCenter
                width: undefined
                Layout.fillWidth: true
                color: MZTheme.theme.bgColor
            }

            MZIconButton {
                id: leaveTutorialBtn
                objectName: "tutorialLeave"
                Keys.enabled: true

                Keys.onTabPressed: pushFocusToTargetElement()
                Keys.onBacktabPressed: pushFocusToTargetElement()

                accessibleName: MZI18n.TutorialPopupLeaveTutorialButton
                Layout.preferredWidth: MZTheme.theme.rowHeight
                Layout.preferredHeight: MZTheme.theme.rowHeight
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                Component.onCompleted: MZTutorial.allowItem(leaveTutorialBtn.objectName)
                buttonColorScheme: MZTheme.theme.iconButtonDarkBackground
                onClicked: openLeaveTutorialPopup()

                Image {
                    anchors.centerIn: parent
                    source: "qrc:/nebula/resources/close-white.svg"
                    sourceSize.height: MZTheme.theme.windowMargin
                    sourceSize.width: MZTheme.theme.windowMargin
                }
            }
        }
    }


    MZSimplePopup {
        property alias primaryButtonText: primaryButton.text
        property var primaryButtonOnClicked: () => {}
        property var secondaryButtonOnClicked: () => {}
        property alias secondaryButtonText: secondaryButton.labelText
        property var _onClosed: () => {}
        property var dismissOnStop: true
        closeButtonObjectName: "vpnPopupCloseButton"

        id: tutorialPopup
        objectName: "tutorialPopup"
        modal: true
        focus: true
        visible: false
        onClosed: _onClosed();
        onOpened: tutorialPopup.forceActiveFocus()
        anchors.centerIn: Overlay.overlay
        imageSize: Qt.size(116, 80)

        Component.onCompleted: {
            [primaryButton.objectName, secondaryButton.objectName, "vpnPopupCloseButton"].forEach(objName => MZTutorial.allowItem(objName));
        }

        title: ""
        description: ""

        buttons: [
            MZButton {
                id: primaryButton
                objectName: "tutorialPopupPrimaryButton"
                text: ""
                Layout.fillWidth: true
                onClicked: tutorialPopup.primaryButtonOnClicked()
            },
            MZLinkButton {
                id: secondaryButton
                objectName: "tutorialPopupSecondaryButton"
                Layout.alignment: Qt.AlignHCenter
                onClicked: tutorialPopup.secondaryButtonOnClicked()
            }
        ]
    }


    function openLeaveTutorialPopup(op = null) {
        tutorialPopup.imageSrc = "qrc:/ui/resources/logo-error.svg";
        tutorialPopup._onClosed = () => {};
        tutorialPopup.primaryButtonOnClicked = () => {
            tutorialPopup.close();
        }

        tutorialPopup.secondaryButtonOnClicked = () => {
            MZGleanDeprecated.recordGleanEventWithExtraKeys("tutorialAborted", {"id": MZTutorial.currentTutorial.id});
            Glean.sample.tutorialAborted.record({ id: MZTutorial.currentTutorial.id });
            tutorialPopup._onClosed = () => {
                if (op !== null) MZTutorial.interruptAccepted(op);
                else MZTutorial.stop();
            }
            tutorialPopup.close();
        }

        tutorialPopup.primaryButtonText = MZI18n.TutorialPopupTutorialLeavePrimaryButtonLabel;
        tutorialPopup.secondaryButtonText = MZI18n.TutorialPopupTutorialFinishedSecondaryButtonLabel;
        tutorialPopup.title = MZI18n.TutorialPopupTutorialLeaveHeadline;
        tutorialPopup.description = MZI18n.TutorialPopupTutorialLeaveSubtitle;
        tutorialPopup.open();
    }


    function openTipsAndTricks() {
        tutorialPopup.close();
        MZNavigator.requestScreen(VPN.ScreenTipsAndTricks);
    }


    function pushFocusToTargetElement() {
        if (targetElement) {
            targetElement.forceActiveFocus();
        }
    }

    function repositionTutorialTooltip() {
        tutorialTooltip.repositionTooltip()
    }

    Connections {
        target: MZTutorial

        function onInterruptRequest(op) {
          openLeaveTutorialPopup(op)
        }

        function onPlayingChanged() {
            if (!MZTutorial.playing) {
                tutorialTooltip.close()
                tutorialTooltip.visible = false
                if (tutorialPopup.opened && tutorialPopup.dismissOnStop) {
                    tutorialPopup.close();
                }
            }
        }

        function onTooltipNeeded(text, targetEl) {
            root.targetElement = targetEl;
            root.tempTooltipText= text;
        }

        function onTutorialCompleted(tutorial) {
            tutorialPopup.imageSrc = "qrc:/ui/resources/logo-success.svg";
            tutorialPopup.primaryButtonOnClicked = () => openTipsAndTricks();
            tutorialPopup.primaryButtonText = MZI18n.TutorialPopupTutorialCompletePrimaryButtonLabel;
            tutorialPopup.secondaryButtonOnClicked = () => tutorialPopup.close();
            tutorialPopup.secondaryButtonText = MZI18n.TutorialPopupTutorialFinishedSecondaryButtonLabel
            tutorialPopup.title =  MZI18n.TutorialPopupTutorialCompleteHeadline;
            tutorialPopup.description = tutorial.completionMessage;
            tutorialPopup._onClosed = () => {};
            tutorialPopup.dismissOnStop = false;
            tutorialPopup.open();
        }

        function onShowWarningNeeded(tutorial) {
            let shouldPlayTutorial = false
            tutorialPopup.imageSrc = "qrc:/ui/resources/logo-warning.svg";
            tutorialPopup.primaryButtonOnClicked = () => {
                shouldPlayTutorial = true
                tutorialPopup.close()
            }
            tutorialPopup.primaryButtonText = MZI18n.GlobalContinue
            tutorialPopup.secondaryButtonOnClicked = () => tutorialPopup.close();
            tutorialPopup.secondaryButtonText = MZI18n.GlobalNoThanks
            tutorialPopup.title = MZI18n.TutorialPopupTutorialWarningTitle;
            tutorialPopup.description = MZI18n.TutorialPopupTutorialWarningDescription
            tutorialPopup._onClosed = () => {
                MZTutorial.stop()
                if(shouldPlayTutorial) {
                    MZTutorial.play(tutorial)
                    MZNavigator.requestScreen(VPN.ScreenHome)
                }
            }
            tutorialPopup.dismissOnStop = false;
            tutorialPopup.open();
        }
    }


    Connections {
        target: window
        function onActiveFocusItemChanged() {
            //https://bugreports.qt.io/browse/QTBUG-59744?gerritIssueType=IssueOnly
            if (!targetElement)
                return

            if (!targetElement.activeFocus && (!targetElement.parent || !targetElement.parent.activeFocus) && !leaveTutorialBtn.activeFocus && !tutorialPopup.opened) {
                leaveTutorialBtn.forceActiveFocus();
            }
        }
    }
}
