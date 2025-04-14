/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import "qrc:/nebula/utils/MZAssetLookup.js" as MZAssetLookup

MZViewBase {
    id: vpnFlickable
    objectName: "firefoxExtensionInfo"

    property string telemetryScreenId: "firefox_extension"


    _menuTitle: MZI18n.SettingsFirefoxExtensionInfo
    _menuOnBackClicked: () => { stackview.pop(null) }
    _viewContentData: ColumnLayout {
        Layout.preferredWidth: parent.width
        Layout.leftMargin: MZTheme.theme.windowMargin * 1.5
        Layout.rightMargin: MZTheme.theme.windowMargin * 1.5
        Layout.alignment: Qt.AlignTop
        spacing: MZTheme.theme.windowMargin * 1.5

        Image {
            Layout.alignment: Qt.AlignHCenter

            source: MZAssetLookup.getLocalizedImageSource("Extension")
            sourceSize.width: 312
            sourceSize.height: 150
        }

        MZInterLabel {
            text: MZI18n.SettingsFirefoxExtensionIntro
            horizontalAlignment: Text.AlignLeft
            Layout.preferredWidth: parent.width
          }

        ColumnLayout {
          spacing: 0
          Layout.preferredWidth: parent.width

          MZBoldInterLabel {
            text: MZI18n.SettingsFirefoxExtensionFirstParagraphHead
            horizontalAlignment: Text.AlignLeft
            font.pixelSize: MZTheme.theme.fontSize
            lineHeight: 24
            Layout.preferredWidth: parent.width
          }

          MZInterLabel {
            text: MZI18n.SettingsFirefoxExtensionFirstParagraphMain
            horizontalAlignment: Text.AlignLeft
            Layout.preferredWidth: parent.width
          }
        }

        ColumnLayout {
          spacing: 0
          Layout.preferredWidth: parent.width

          MZBoldInterLabel {
            text: MZI18n.SettingsFirefoxExtensionSecondParagraphHead
            horizontalAlignment: Text.AlignLeft
            font.pixelSize: MZTheme.theme.fontSize
            lineHeight: 24
            Layout.preferredWidth: parent.width
          }

          MZInterLabel {
            text: MZI18n.SettingsFirefoxExtensionSecondParagraphMain
            horizontalAlignment: Text.AlignLeft
            Layout.preferredWidth: parent.width
          }
        }

        ColumnLayout {
          spacing: 10
          MZButton {
              id: extensionButton

              text: MZI18n.SettingsFirefoxExtensionMainButton
              Layout.alignment: Qt.AlignHCenter
              Layout.fillWidth: true
              width: undefined
              onClicked: {
                  Glean.interaction.getTheExtensionSelected.record({
                      screen: root.telemetryScreenId,
                  });

                  MZUrlOpener.openUrl("https://addons.mozilla.org/firefox/addon/mozilla-vpn-extension/?utm_medium=mozilla-vpn-client&utm_source=vpn-settings&utm_campaign=evergreen&utm_content=firefox-extension")
              }
          }

          MZLinkButton {
              id: firefoxButton
              labelText: MZI18n.SettingsFirefoxExtensionFirefoxButton
              onClicked: {
                Glean.interaction.downloadFirefoxSelected.record({
                      screen: root.telemetryScreenId,
                  });
                  MZUrlOpener.openUrl("https://www.mozilla.org/firefox/?utm_medium=mozilla-vpn-client&utm_source=vpn-settings&utm_campaign=evergreen&utm_content=firefox-extension")
              }
              Layout.alignment: Qt.AlignHCenter
          }
        }

    }

    Component.onCompleted: {
        Glean.impression.firefoxExtensionScreen.record({
            screen: telemetryScreenId,
        });
    }
}
