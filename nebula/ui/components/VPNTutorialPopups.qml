/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import compat 0.1


Item {
    /*
        TODOs

        - Handle banner alerts
        - Handle conflicts with update required

        - Should tutorials timeout?
        - Handle device orientation changes
        - Better UI in landscape mode
        - Respond to targetElement position changes
        - Animation polish on tooltip open

        - Update event filter
            - Add touch events for mobile
            - Restrict wheel events
            - Consider restricting key commands to allowed items

        - Second tutorial nits:
            - Toggle checkbox status with space bar does not trigger next step
            - System preferences is not immediately focused, it's possible to tab to the back arrow and get in a weird state
    */


    // targetElement is set to `parent` here to get around `Cannot call method ... of undefined` warnings
    // and is reset before the tutorial is opened in onTooltipNeeded()
    property var targetElement: parent
    onTargetElementChanged: pushFocusToEmitter()

    id: root

    anchors.fill: parent
    visible: tutorialTooltip.visible || tutorialPopup.opened

    Popup {
        property alias tooltipText: tooltipText.text
        property int notchHeight: VPNTheme.theme.windowMargin
        property bool tooltipPositionedAboveEmitter: false

        id: tutorialTooltip

        closePolicy: Popup.NoAutoClose
        onClipChanged: VPNTutorial.stop();
        verticalPadding: VPNTheme.theme.windowMargin
        horizontalPadding: VPNTheme.theme.windowMargin
        focus: true

        y: {
            if (targetElement && typeof(targetElement) !== "undefined") {
               const windowHeight = window.height;
               const targetElementDistanceFromTop = targetElement.mapToItem(window.contentItem, 0, 0).y

               if (targetElementDistanceFromTop + targetElement.height + tutorialTooltip.implicitHeight > windowHeight) {
                   tooltipPositionedAboveEmitter = true;
                   return targetElementDistanceFromTop - targetElement.height - notchHeight * 1.5;
               }

               tooltipPositionedAboveEmitter = false;
               return targetElement.mapToItem(window.contentItem, 0, 0).y + targetElement.height + notchHeight;
           }
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
                color: VPNTheme.colors.grey60
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
                border.color: VPNTheme.colors.purple80
            }

            Rectangle {
                property int tooltipOffset: tutorialTooltip.notchHeight / 4
                id: notch
                height: tutorialTooltip.notchHeight
                width: tutorialTooltip.notchHeight
                radius: 2
                color: backgroundRect.color
                y: tutorialTooltip.tooltipPositionedAboveEmitter ? tutorialTooltip.height - (tutorialTooltip.notchHeight - tooltipOffset) : tooltipOffset * -1
                rotation: 45
                anchors.left: parent.left
                anchors.leftMargin: if (targetElement && typeof(targetElement) !== undefined) { (targetElement.mapToItem(window.contentItem, 0, 0).x + (targetElement.width / 2)) - tutorialTooltip.x - tutorialTooltip.notchHeight/2 }
            }
        }

        contentItem: RowLayout {
           id: content
           spacing: VPNTheme.theme.windowMargin / 2

           VPNTextBlock {
                id: tooltipText
                text: ""
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                verticalAlignment: Text.AlignVCenter
                width: undefined
                Layout.fillWidth: true
                color: VPNTheme.theme.bgColor
            }

            VPNIconButton {
                id: leaveTutorialBtn
                objectName: "tutorialLeave"
                Keys.enabled: true

                Keys.onTabPressed: pushFocusToEmitter()
                Keys.onBacktabPressed: pushFocusToEmitter()

                accessibleName: VPNl18n.TutorialPopupLeaveTutorialButton
                Layout.preferredWidth: VPNTheme.theme.rowHeight
                Layout.preferredHeight: VPNTheme.theme.rowHeight
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                Component.onCompleted: VPNTutorial.allowItem(leaveTutorialBtn.objectName)
                buttonColorScheme: VPNTheme.theme.iconButtonDarkBackground
                onClicked: openLeaveTutorialPopup()

                Image {
                    anchors.centerIn: parent
                    source: "qrc:/nebula/resources/close-white.svg"
                    sourceSize.height: VPNTheme.theme.windowMargin
                    sourceSize.width: VPNTheme.theme.windowMargin
                }
            }
        }
    }


    VPNPopup {
        property alias _popupImgSrc: popupImage.source
        property alias _popupHeadline: popupHeadline.text
        property alias _popupSubtitle: popupSubtitle.text
        property alias _primaryButtonText: primaryButton.text
        property var _primaryButtonOnClicked: () => {}
        property var _secondaryButtonOnClicked: () => {}
        property var _onClosed: () => {}

        id: tutorialPopup

        anchors.centerIn: parent
        width: Math.min(parent.width, VPNTheme.theme.maxHorizontalContentWidth)
        height: Math.min(parent.height - VPNTheme.theme.windowMargin * 2, implicitHeight + VPNTheme.theme.windowMargin * 2)
        maxWidth: Math.min(parent.width - (VPNTheme.theme.windowMargin * 2), VPNTheme.theme.maxHorizontalContentWidth)
        modal: true
        focus: true
        visible: false
        onClosed: _onClosed();

        Component.onCompleted: {
            [primaryButton.objectName, secondaryButton.objectName, "vpnPopupCloseButton"].forEach(objName => VPNTutorial.allowItem(objName));
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
                }

                VPNLinkButton {
                    id: secondaryButton
                    objectName: "tutorialPopupSecondaryButton"
                    labelText: VPNl18n.TutorialPopupSecondaryButtonLabel
                    Layout.alignment: Qt.AlignHCenter
                    onClicked: tutorialPopup._secondaryButtonOnClicked()
                }
            }
    }


     // Called from VPNButtonBase.qml
    function forceTooltipFocus(event) {
        if (!tutorialPopup.opened) {
            tutorialTooltip.forceActiveFocus();
            event.accepted = true;
        } else {
            event.accepted = false;
        }
    }


    function leaveTutorial() {
        VPNTutorial.stop();
        tutorialPopup.close();
    }


    function openLeaveTutorialPopup(callback = () => {}) {
        tutorialPopup._popupImgSrc = "qrc:/ui/resources/logo-error.svg";
        tutorialPopup._primaryButtonOnClicked = () => tutorialPopup.close();
        tutorialPopup._secondaryButtonOnClicked = () => {
            tutorialPopup._onClosed = () => callback()
            leaveTutorial();
        }
        tutorialPopup._primaryButtonText = VPNl18n.TutorialPopupTutorialLeavePrimaryButtonLabel;
        tutorialPopup._popupHeadline = VPNl18n.TutorialPopupTutorialLeaveHeadline;
        tutorialPopup._popupSubtitle = VPNl18n.TutorialPopupTutorialLeaveSubtitle;
        tutorialPopup.open();
    }


    function openTipsAndTricks() {
        tutorialPopup.close();
        VPN.settingsNeeded();
        const settingsViewInMainStack = mainStackView.find((view) => { return view.objectName === "settings" });
        settingsViewInMainStack._openTipsAndTricks();
    }


    function pushFocusToEmitter() {
        if (targetElement) {
            targetElement.forceActiveFocus();
        }
    }

    Connections {
        target: VPNTutorial

        function onTooltipNeeded(text, targetEl) {
            root.targetElement = targetEl;
            tutorialTooltip.tooltipText = text;
            tutorialTooltip.open();
        }

        function onTutorialCompleted(localizedTutorialCompletedCopy) {
            tutorialPopup._popupImgSrc = "qrc:/ui/resources/logo-success.svg";
            tutorialPopup._primaryButtonOnClicked = () => openTipsAndTricks();
            tutorialPopup._primaryButtonText = VPNl18n.TutorialPopupTutorialCompletePrimaryButtonLabel;
            tutorialPopup._secondaryButtonOnClicked = () => tutorialPopup.close();
            tutorialPopup._popupHeadline =  VPNl18n.TutorialPopupTutorialCompleteHeadline;
            tutorialPopup._popupSubtitle = localizedTutorialCompletedCopy;
            tutorialPopup._onClosed = () => {};
            tutorialPopup.open();
        }
    }
}

