/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import compat 0.1
import "qrc:/nebula/utils/MZAssetLookup.js" as MZAssetLookup

// A non-modal speech bubble that points down toward the messages nav button.
// Position this item so that its bottom edge aligns with the tip of the caret
// (i.e. set y = navbar.y - height).
Item {
    id: root

    property string messageText: ""
    property string promotedAddonId: ""

    signal dismissed()
    signal promoteMessage(string addonId)

    readonly property int _caretWidth: 16
    readonly property int _caretHeight: 8
    readonly property int _bubblePadding: MZTheme.theme.windowMargin / 2

    width: bubble.width
    height: bubble.height + _caretHeight

    opacity: 0
    visible: false

    function show() {
        root.visible = true;
        fadeIn.restart();
    }

    function hide() {
        if (!root.visible) return;
        fadeOut.restart();
    }

    MZFilterProxyModel {
        id: addonsToPromote
        source: MZAddonManager
        filterCallback: obj => { 
          return obj.addon.type === "message" && obj.addon.isNotified && obj.addon.promoText && obj.addon.promoText.length > 0;
        }
    }

    Repeater {
        model: MZAddonManager
        delegate: Item {
          Connections {
              target: addon
              function onStatusChanged() {
                  addonsToPromote.recalculate();
              }
          }
        }
    }


    Connections {
      target: window
      function onShowServerList(isImmediate) {
          messageAddonPopover.hide();
      }
    }  

    Connections {
        target: MZNavigator
        function onCurrentComponentChanged() {
            if (MZNavigator.screen !== VPN.ScreenHome) {
                root.hide();
            }
            maybeShowMessagePopover();
        }
    }

    // for when it's loading the addons
    Connections {                                                                                                                                                                   
      target: addonsToPromote                                                                                                                                               
      function onCountChanged() {
          maybeShowMessagePopover();                                                                                                                                        
      }                                                                                                                                                                           
    }

    // Handle the case where VPN state becomes StateMain after the screen
    // has already transitioned to ScreenHome (e.g. at app startup).
    Connections {
        target: VPN
        function onStateChanged() {
            maybeShowMessagePopover();
        }
    }

    function maybeShowMessagePopover() {
        if (MZNavigator.screen !== VPN.ScreenHome ||
          VPN.state !== VPN.StateMain) {
            console.log("[MessageAddonPopover] wrong conditions, not showing");
            return;
        }

        const threeDays = 1000 * 60 * 60 * 24 * 3;
        const thirtySeconds = 1000 * 30;
        const minimumTime = MZSettings.useShortAddonPromoTime ? thirtySeconds : threeDays;
        const timeSinceLastPromo = new Date() - MZSettings.addonPromoLastShown;
        const withinMinimumTime = timeSinceLastPromo < minimumTime;
        if (withinMinimumTime) {
            console.log("[MessageAddonPopover] promotion was recently shown, skipping");
            return;
        }

        if (root.visible) {
            console.log("[MessageAddonPopover] already visible, skipping");
            return;
        }

        if (addonsToPromote.count === 0) {
            console.log("[MessageAddonPopover] no eligible addon found");
            return;
        }
        var addon = addonsToPromote.get(0).addon;

        if (!addon.promoText) {
            console.log("[MessageAddonPopover] promo text is empty, skipping");
            return;
        }

        console.log("[MessageAddonPopover] showing " + addon.id + " with: " + addon.promoText);
        root.promotedAddonId = addon.id;
        root.messageText = addon.promoText;
        root.show();
        MZSettings.addonPromoLastShown = new Date()
        addon.markAsPromoShown();
    }

    NumberAnimation on opacity {
        id: fadeIn
        running: false
        from: 0; to: 1
        duration: 180
        easing.type: Easing.InOutQuad
    }

    NumberAnimation on opacity {
        id: fadeOut
        running: false
        from: 1; to: 0
        duration: 120
        easing.type: Easing.InOutQuad
        onStopped: {
            root.visible = false;
            root.dismissed();
        }
    }

    Rectangle {
        id: bubble

        width: Math.min(contentRow.implicitWidth + _bubblePadding * 3,
                  window.width - MZTheme.theme.windowMargin * 4)
        height: contentRow.implicitHeight + _bubblePadding * 2
        radius: 8
        color: MZTheme.colors.bgColorStronger

        MouseArea {
            anchors.fill: parent
            onClicked: {
              root.hide()
              window.promotedAddonId = root.promotedAddonId;
              MZNavigator.requestScreen(VPN.ScreenMessaging)
              endAddonPromotionTimer.restart()
            }
        }

        Timer {
          id: endAddonPromotionTimer
          interval: 1200
          onTriggered: window.promotedAddonId = ""
        }

        MZDropShadow {
            anchors.fill: shadowSource
            source: shadowSource
            transparentBorder: true
            verticalOffset: 2
            radius: 8
            color: MZTheme.colors.dropShadow
            opacity: 0.2
            cached: true
            z: -1
        }

        Rectangle {
            id: shadowSource
            anchors.fill: parent
            radius: parent.radius
            visible: false
            z: -1
        }

        RowLayout {
            id: contentRow

            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
                leftMargin: _bubblePadding * 2
                rightMargin: _bubblePadding
                topMargin: _bubblePadding
            }
            spacing: _bubblePadding

            Text {
                id: label
                text: root.messageText
                color: MZTheme.colors.fontColorDark
                font.pixelSize: MZTheme.theme.fontSize
                wrapMode: Text.WordWrap
                Accessible.ignored: !root.visible
            }

            MZIconButton {
                id: closeBtn
                Layout.preferredWidth: MZTheme.theme.iconSize + _bubblePadding
                Layout.preferredHeight: MZTheme.theme.iconSize + _bubblePadding
                Layout.alignment: Qt.AlignTop
                accessibleName: MZI18n.GlobalClose
                objectName: "messageAddonPopover-closeButton"
                buttonColorScheme: MZTheme.colors.iconButtonLightBackground

                onClicked: root.hide()

                Image {
                    anchors.centerIn: parent
                    source: MZAssetLookup.getImageSource("CloseDark")
                    sourceSize.width: MZTheme.theme.iconSize
                    sourceSize.height: MZTheme.theme.iconSize
                    fillMode: Image.PreserveAspectFit
                }
            }
        }
    }

    // Downward-pointing caret connecting the bubble to the nav button
    MZDropShadow {
        anchors.fill: caret
        source: caret
        transparentBorder: true
        verticalOffset: 2
        radius: 8
        color: MZTheme.colors.dropShadow
        opacity: 0.2
        cached: true
        z: -1
    }

    Canvas {
        id: caret

        width: _caretWidth
        height: _caretHeight
        anchors.top: bubble.bottom
        anchors.horizontalCenter: bubble.horizontalCenter

        // this allows the caret to change color when theme changes
        property color caretColor: MZTheme.colors.bgColorStronger                                                                                                                       
        onCaretColorChanged: requestPaint() 

        onPaint: {
            var ctx = getContext("2d");
            ctx.clearRect(0, 0, width, height);
            ctx.fillStyle = caretColor;
            ctx.beginPath();
            ctx.moveTo(0, 0);
            ctx.lineTo(width, 0);
            ctx.lineTo(width / 2, height);
            ctx.closePath();
            ctx.fill();
        }
    }
}
