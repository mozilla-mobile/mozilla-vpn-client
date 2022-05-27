/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import compat 0.1
import components 0.1

import QtQuick.Shapes 1.13


Item {
    id: root

    /*
        TODOs

        - Final copy
        - Add intro poup content
        - Handle interruptions
            - Opening Preferences from system tray
            - Opening contact us from system tray
            - Opening logs from system tray
            - Unsecure network alert
            - Cannot connect to server
            - Banner alerts

        - Should tutorials timeout?

        - Handle device orientation changes
        - Handle console warnings
        - Better UI in landscape mode

        - Update event filter
            - Consider restricting scrolling
            - Consider restricting key commands

        Second tutorial nits:
            - Toggle checkbox status with space bar does not trigger next step
            - System preferences is not immediately focused, it's possible to tab to the back arrow and get in a weird state
    */


    // targetElement is set to `parent` here to get around `Cannot call method ... of undefined` warnings
    // and is reset before the tutorial is opened in onTooltipNeeded()
    property var targetElement: parent
    onTargetElementChanged: targetElement.forceActiveFocus()

    function pushFocusToEmitter() {
        targetElement.forceActiveFocus();
        return;
    }

    function forceTooltipFocus(event) {
        if (!tutorialPopup.opened) {
            tutorialTooltip.forceActiveFocus();
            event.accepted = true;
        } else {
            event.accepted = false;
        }
    }

    anchors.fill: parent
    visible: tutorialTooltip.visible || tutorialPopup.visible

    Popup {
        property alias tooltipText: tooltipText.text
        property int notchHeight: VPNTheme.theme.windowMargin
        property bool tooltipPositionedAboveEmitter: false
        property int yChangeOnOpen: 10

        id: tutorialTooltip

        closePolicy: Popup.NoAutoClose
        onClipChanged: VPNTutorial.stop();
        verticalPadding: VPNTheme.theme.windowMargin
        horizontalPadding: VPNTheme.theme.windowMargin

        y: if (typeof(targetElement.height) !== "undefined") {
               const windowHeight = window.height;
               const targetElementDistanceFromTop = targetElement.mapToItem(window.contentItem, 0, 0).y

               if (targetElementDistanceFromTop + targetElement.height + tutorialTooltip.implicitHeight > windowHeight) {
                   tooltipPositionedAboveEmitter = true;
                   return targetElementDistanceFromTop - targetElement.height - notchHeight * 1.5;
               }
               tooltipPositionedAboveEmitter = false;
               return targetElement.mapToItem(window.contentItem, 0, 0).y + targetElement.height + notchHeight;
           }



        width: root.width - VPNTheme.theme.windowMargin * 2
        visible: VPNTutorial.tooltipShown
        x: VPNTheme.theme.windowMargin

        background: Rectangle {
            id: backgroundRect
            color:  VPNTheme.colors.primary
            radius: 4

            VPNDropShadow {
                id: notchShadow
                source: notchOutline
                anchors.fill: notchOutline
                transparentBorder: shadow.transparentBorder
                radius: shadow.radius
                color: shadow.color
                cached: true
                rotation: notch.rotation
            }

            VPNDropShadow {
                id: shadow
                source: tooltipOutline
                anchors.fill: tooltipOutline
                transparentBorder: true
                radius: 7.5
                color: "#0C0C0D"
                cached: true
            }

            Rectangle {
                id: notchOutline
                height: tutorialTooltip.notchHeight + 2
                width: tutorialTooltip.notchHeight + 2
                radius: notch.radius
                color: backgroundRect.color
                rotation: notch.rotation
                anchors.centerIn: notch
                border.width: tooltipOutline.border.width
                border.color: tooltipOutline.border.color
            }

            Rectangle {
                id: tooltipOutline
                color: parent.color
                radius: parent.radius
                anchors.fill: parent
                border.width: 1
                border.color: "#45278D"
            }

            Rectangle {
                id: notch
                height: tutorialTooltip.notchHeight
                width: tutorialTooltip.notchHeight
                radius: 2
                color: backgroundRect.color
                y: tutorialTooltip.tooltipPositionedAboveEmitter ? tutorialTooltip.height - (tutorialTooltip.notchHeight - 4) : -4
                rotation: 45
                x: if (typeof(targetElement.x) !== "undefined") { (targetElement.mapToItem(window.contentItem, 0, 0).x + (targetElement.width / 2)) - tutorialTooltip.x - tutorialTooltip.notchHeight/2 }
            }
        }

        PropertyAnimation on y {
            duration: 100
        }

       contentItem: RowLayout {
           spacing: VPNTheme.theme.windowMargin / 2

           VPNTextBlock {
                // todo: sensible text wrapping
                id: tooltipText
                text: "This is text that has to wrap and wrap anad wrap and wrap"
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                verticalAlignment: Text.AlignVCenter
                width: undefined
                Layout.fillWidth: true
                color: "white"
            }

            VPNIconButton {
                id: exitTutorial
                objectName: "tutorialExit"
                Keys.enabled: true

                Keys.onTabPressed: pushFocusToEmitter()
                Keys.onBacktabPressed: pushFocusToEmitter()

                accessibleName: "todo"
                Layout.preferredWidth: VPNTheme.theme.rowHeight
                Layout.preferredHeight: VPNTheme.theme.rowHeight
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                Component.onCompleted: VPNTutorial.allowItem("tutorialExit")
                buttonColorScheme: VPNTheme.theme.iconButtonDarkBackground
                onClicked: openQuitTutorialPopup()

                Image {
                    anchors.centerIn: parent
                    source: "qrc:/nebula/resources/close-white.svg"
                    sourceSize.height: VPNTheme.theme.windowMargin
                    sourceSize.width: VPNTheme.theme.windowMargin
                }
            }
        }

        Connections {

            target: VPNTutorial
            function onTooltipNeeded(text, targetEl) {
                tutorialTooltip.tooltipText = qsTrId(text));
                root.targetElement = targetEl;
                tutorialTooltip.open();
            }

            function onTutorialCompleted(text) {
                openTutorialCompletedPopup(text);
            }

            function quitTutorial() {
                openQuitTutorialPopup();
            }
        }
    }

    function closeTutorial() {
        tutorialPopup.close();
        VPNTutorial.stop();
    }

    function openTipsAndTricks() {
        tutorialPopup.close()
        VPN.settingsNeeded();
        const settingsViewInMainStack = mainStackView.find((view) => { return view.objectName === "settings" });
        settingsViewInMainStack._openTipsAndTricks();
    }

    function tutorialIntro() {
        // todo
    }

    function tutorialOnSomeError() {
        // todo
    }

    function openQuitTutorialPopup() {
        tutorialPopup._popupImgSrc = "qrc:/ui/resources/logo-error.svg";
        tutorialPopup._primaryButtonOnClicked = () => { closeTutorial() };
        tutorialPopup._primaryButtonText = "Quit";
        tutorialPopup._secondaryButtonText = "Resume tutorial";
        tutorialPopup._popupHeadline = "Quit tutorial?";
        tutorialPopup._popupSubtitle = VPNl18n.TutorialPopupTutorialExitSubtitle;
        tutorialPopup.open();
    }

    function openTutorialCompletedPopup(localizedTutorialCompletedCopy) {
        tutorialPopup._popupImgSrc = "qrc:/ui/resources/logo-success.svg";
        tutorialPopup._primaryButtonOnClicked = () => { openTipsAndTricks() /* TODO: Open tips and tricks */ };
        tutorialPopup._primaryButtonText = VPNl18n.TutorialTutorialCompletePrimaryButtonLabel;
        tutorialPopup._secondaryButtonText = VPNl18n.TutorialPopupTutorialCompleteSecondaryButtonLabel;
        tutorialPopup._popupHeadline =  VPNl18n.TutorialPopupTutorialCompleteHeadline;
        tutorialPopup._popupSubtitle = localizedTutorialCompletedCopy;
        tutorialPopup.open();
    }

    VPNPopup {
        property alias _popupImgSrc: popupImage.source
        property alias _popupHeadline: popupHeadline.text
        property alias _popupSubtitle: popupSubtitle.text
        property alias _primaryButtonText: primaryButton.text
        property var _primaryButtonOnClicked
        property alias _secondaryButtonText: secondaryButton.labelText

        id: tutorialPopup

        anchors.centerIn: parent
        width: parent.width
        height: Math.min(parent.height - VPNTheme.theme.windowMargin * 2, implicitHeight + VPNTheme.theme.windowMargin * 2)
        maxWidth: Math.min(parent.width - (VPNTheme.theme.windowMargin * 2), VPNTheme.theme.maxHorizontalContentWidth)
        modal: true
        focus: true

        Component.onCompleted: {
            VPNTutorial.allowItem(primaryButton.objectName);
            VPNTutorial.allowItem(secondaryButton.objectName)
            VPNTutorial.allowItem("vpnPopupCloseButton");
        }

        _popupContent:
            ColumnLayout {
                id: tutorialPopupContent
                spacing: VPNTheme.theme.vSpacing

                Image {
                    id: popupImage
                    source: ""
                    antialiasing: true
                    sourceSize.height: 80
                    Layout.alignment: Qt.AlignHCenter
                }

                VPNHeadline {
                    id: popupHeadline
                    width: undefined
                    Layout.fillWidth: true
                    text: ""
                }

                VPNTextBlock {
                    id: popupSubtitle
                    horizontalAlignment: Text.AlignHCenter
                    Layout.preferredWidth: parent.width
                    Layout.fillWidth: true
                    text: ""
                }

                VPNButton {
                    id: primaryButton
                    objectName: "tutorialPopupPrimaryButton"
                    text: ""
                    onClicked: tutorialPopup._primaryButtonOnClicked()
                    KeyNavigation.tab: secondaryButton
                }

                VPNLinkButton {
                    id: secondaryButton
                    objectName: "secondaryButton"
                    labelText: ""
                    Layout.alignment: Qt.AlignHCenter
                    onClicked: tutorialPopup.close()
                }
            }
    }
}

