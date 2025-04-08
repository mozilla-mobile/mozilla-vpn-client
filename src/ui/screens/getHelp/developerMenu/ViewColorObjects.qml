
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1

import "qrc:/nebula/utils/MZAssetLookup.js" as MZAssetLookup

MZViewBase {
    id: root

    _menuTitle: "Color exploration"
    _viewContentData: ColumnLayout {
          Layout.leftMargin: 20
          Layout.rightMargin: 20
          spacing: 20

          MZTextBlock {
              text: "Info cards use the `[type]Accent` color for their stripe"
              Layout.fillWidth: true
          }

          MZInformationCard {
              width: Math.min(window.width - MZTheme.theme.windowMargin * 2, MZTheme.theme.navBarMaxWidth)
              cardType: MZInformationCard.CardType.Info
              anchors.horizontalCenter: parent.horizontalCenter
              implicitHeight: textBlock.height + MZTheme.theme.windowMargin * 2
              
              _infoContent: MZTextBlock {
                  id: textBlock
                  Layout.fillWidth: true

                  text: "Info card with normal colors"
                  verticalAlignment: Text.AlignVCenter
              }
          }

          MZInformationCard {
              width: Math.min(window.width - MZTheme.theme.windowMargin * 2, MZTheme.theme.navBarMaxWidth)
              cardType: MZInformationCard.CardType.Success
              anchors.horizontalCenter: parent.horizontalCenter
              implicitHeight: textBlock.height + MZTheme.theme.windowMargin * 2
              
              _infoContent: MZTextBlock {
                  // id: textBlock
                  Layout.fillWidth: true

                  text: "Info card with success colors"
                  verticalAlignment: Text.AlignVCenter
              }
          }

          MZInformationCard {
              width: Math.min(window.width - MZTheme.theme.windowMargin * 2, MZTheme.theme.navBarMaxWidth)
              cardType: MZInformationCard.CardType.Warning
              anchors.horizontalCenter: parent.horizontalCenter
              implicitHeight: textBlock.height + MZTheme.theme.windowMargin * 2
              
              _infoContent: MZTextBlock {
                  // id: textBlock
                  Layout.fillWidth: true

                  text: "Info card with warning colors"
                  verticalAlignment: Text.AlignVCenter
              }
          }

          MZInformationCard {
              width: Math.min(window.width - MZTheme.theme.windowMargin * 2, MZTheme.theme.navBarMaxWidth)
              cardType: MZInformationCard.CardType.Error
              anchors.horizontalCenter: parent.horizontalCenter
              implicitHeight: textBlock.height + MZTheme.theme.windowMargin * 2
              
              _infoContent: MZTextBlock {
                  // id: textBlock
                  Layout.fillWidth: true

                  text: "Info card with error colors"
                  verticalAlignment: Text.AlignVCenter
              }
          }

          Rectangle {
              Layout.fillWidth: true
              Layout.preferredHeight: 10

              color: MZTheme.colors.divider
          }

          MZTextBlock {
              text: "Alerts use the `[type]Accent` colors, among many others"
              Layout.fillWidth: true
          }

          MZButton {
              id: errorAlert
              text: "Click for sample error alert"
              onClicked: MZErrorHandler.requestAlert(MZErrorHandler.GeoIpRestrictionAlert)
          }

          MZButton {
              id: warningAlert
              text: "Click for sample warning alert"
              onClicked: MZErrorHandler.requestAlert(MZErrorHandler.SampleWarningAlert)
          }

          MZButton {
              id: infoAlert
              text: "Click for sample info alert"
              onClicked: MZErrorHandler.requestAlert(MZErrorHandler.SampleInfoAlert)
          }

          MZButton {
              id: successAlert
              text: "Click for sample success alert"
              onClicked: MZErrorHandler.requestAlert(MZErrorHandler.LogoutAlert)
          }

          Rectangle {
              Layout.fillWidth: true
              Layout.preferredHeight: 10

              color: MZTheme.colors.divider
          }

          MZTextBlock {
              text: "Badges use the `[type]Main` and `[type]Background colors"
              Layout.fillWidth: true
          }

          MZBadge {
              badgeLabel.width: badgeLabel.implicitWidth
              badgeType: {
                      'badgeText': "Error badge",
                      'badgeTextColor': MZTheme.colors.errorText,
                      'badgeBackgroundColor': MZTheme.colors.errorBackground
                  };
          }

          MZBadge {
              badgeLabel.width: badgeLabel.implicitWidth
              badgeType: {
                      'badgeText': "Warning badge",
                      'badgeTextColor': MZTheme.colors.warningText,
                      'badgeBackgroundColor': MZTheme.colors.warningBackground
                  };
          }

          MZBadge {
              badgeLabel.width: badgeLabel.implicitWidth
              badgeType: {
                      'badgeText': "Success badge",
                      'badgeTextColor': MZTheme.colors.successText,
                      'badgeBackgroundColor': MZTheme.colors.successBackground
                  };
          }

          MZBadge {
              badgeLabel.width: badgeLabel.implicitWidth
              badgeType: {
                      'badgeText': "Normal badge",
                      'badgeTextColor': MZTheme.colors.normalLevelText,
                      'badgeBackgroundColor': MZTheme.colors.normalLevelBackground
                  };
          }

          Rectangle {
              Layout.fillWidth: true
              Layout.preferredHeight: 10

              color: MZTheme.colors.divider
          }

          MZIcon {
            source: MZAssetLookup.getImageSource("PaymentAmex")
            sourceSize.height: MZTheme.theme.iconSizeSmall * 1.5
            sourceSize.width: MZTheme.theme.iconSizeSmall * 1.5
          }

          MZIcon {
            source: MZAssetLookup.getImageSource("PaymentDiners")
            sourceSize.height: MZTheme.theme.iconSizeSmall * 1.5
            sourceSize.width: MZTheme.theme.iconSizeSmall * 1.5
          }

          MZIcon {
            source: MZAssetLookup.getImageSource("PaymentDiscover")
            sourceSize.height: MZTheme.theme.iconSizeSmall * 1.5
            sourceSize.width: MZTheme.theme.iconSizeSmall * 1.5
          }

          MZIcon {
            source: MZAssetLookup.getImageSource("PaymentJcb")
            sourceSize.height: MZTheme.theme.iconSizeSmall * 1.5
            sourceSize.width: MZTheme.theme.iconSizeSmall * 1.5
          }

          MZIcon {
            source: MZAssetLookup.getImageSource("PaymentApple")
            sourceSize.height: MZTheme.theme.iconSizeSmall * 1.5
            sourceSize.width: MZTheme.theme.iconSizeSmall * 1.5
          }

          MZIcon {
            source: MZAssetLookup.getImageSource("PaymentGoogle")
            sourceSize.height: MZTheme.theme.iconSizeSmall * 1.5
            sourceSize.width: MZTheme.theme.iconSizeSmall * 1.5
          }

          MZIcon {
            source: MZAssetLookup.getImageSource("PaymentMastercard")
            sourceSize.height: MZTheme.theme.iconSizeSmall * 1.5
            sourceSize.width: MZTheme.theme.iconSizeSmall * 1.5
          }

          MZIcon {
            source: MZAssetLookup.getImageSource("PaymentPayPal")
            sourceSize.height: MZTheme.theme.iconSizeSmall * 1.5
            sourceSize.width: MZTheme.theme.iconSizeSmall * 1.5
          }

          MZIcon {
            source: MZAssetLookup.getImageSource("PaymentUnionPay")
            sourceSize.height: MZTheme.theme.iconSizeSmall * 1.5
            sourceSize.width: MZTheme.theme.iconSizeSmall * 1.5
          }

          MZIcon {
            source: MZAssetLookup.getImageSource("PaymentVisa")
            sourceSize.height: MZTheme.theme.iconSizeSmall * 1.5
            sourceSize.width: MZTheme.theme.iconSizeSmall * 1.5
          }

          MZIcon {
            source: MZAssetLookup.getImageSource("PaymentUnbranded")
            sourceSize.height: MZTheme.theme.iconSizeSmall * 1.5
            sourceSize.width: MZTheme.theme.iconSizeSmall * 1.5
          }

          Rectangle {
              Layout.fillWidth: true
              Layout.preferredHeight: 10

              color: MZTheme.colors.divider
          }

          MZButton {
              anchors.left: parent.left
              anchors.right: parent.right

              implicitHeight: MZTheme.theme.rowHeight
              text: "Activate `ButtonLoader`"

              onClicked: { loaderVisible = true }
          }
      }
}
