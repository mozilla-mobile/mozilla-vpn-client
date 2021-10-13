/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
// import QtGraphicalEffects 1.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import themes 0.1

// TODO: This filename should be changed to 'ViewPreferences.qml' after the 2.5 release
// The legacy name has been kept to prevent the creation of additional strings

Item {
    property string _startAtBootTitle: ""
    property string _notificationsTitle: ""
    property string _languageTitle: ""
    property string _menuTitle

    id: root

    VPNFlickable {
        id: vpnFlickable
        objectName: "settingsPreferencesView"
        anchors.top: parent.top
        anchors.topMargin: Theme.menuHeight
        anchors.left: root.left
        anchors.right: root.right
        height: root.height - menu.height
        flickContentHeight: col.height
        interactive: flickContentHeight > height

        Column {
            id: col
            anchors.top: parent.top
            anchors.topMargin: Theme.windowMargin
            anchors.left: parent.left
            anchors.right: parent.right
            spacing: Theme.windowMargin
            width: parent.width - Theme.windowMargin

            VPNCheckBoxRow {
                id: startAtBootCheckBox
                objectName: "settingStartAtBoot"

                labelText: _startAtBootTitle
                subLabelText: ""
                isChecked: VPNSettings.startAtBoot
                isEnabled: true
                showDivider: false
                width: parent.width - Theme.windowMargin
                onClicked: VPNSettings.startAtBoot = !VPNSettings.startAtBoot
                visible: VPNFeatureList.get("startOnBoot").isSupported

            }

            VPNVerticalSpacer {
                height: 1
                width: parent.width
            }

            VPNCheckBoxRow {
                id: dataCollection
                objectName: "dataCollection"
                width: parent.width - Theme.windowMargin
                anchors.left: parent.left
                anchors.right: parent.right

                //% "Data collection and use"
                labelText: qsTrId("vpn.settings.dataCollection")
                //% "Allow Mozilla VPN to send technical data to Mozilla"
                subLabelText: qsTrId("vpn.settings.dataCollection.description")
                isChecked: VPNSettings.gleanEnabled
                onClicked: {
                    VPNSettings.gleanEnabled = !VPNSettings.gleanEnabled
               }
            }

            Column {
                spacing: Theme.windowMargin / 2
                width: parent.width
                VPNSettingsItem {
                    objectName: "settingsNotifications"
                    anchors.left: parent.left
                    anchors.right: parent.right

                    settingTitle: _notificationsTitle
                    imageLeftSrc: "qrc:/ui/resources/settings/notifications.svg"
                    imageRightSrc: "qrc:/ui/resources/chevron.svg"
                    onClicked: settingsStackView.push("qrc:/ui/settings/ViewNotifications.qml")
                    visible: VPNFeatureList.get("captivePortal").isSupported || VPNFeatureList.get("unsecuredNetworkNotification").isSupported || VPNFeatureList.get("notificationControl").isSupported
                    width: parent.width - Theme.windowMargin
                }

                VPNSettingsItem {
                    objectName: "settingsLanguages"

                    anchors.left: parent.left
                    anchors.right: parent.right

                    settingTitle: _languageTitle
                    imageLeftSrc: "qrc:/ui/resources/settings/language.svg"
                    imageRightSrc: "qrc:/ui/resources/chevron.svg"
                    onClicked: settingsStackView.push("qrc:/ui/settings/ViewLanguage.qml")
                    visible: VPNLocalizer.hasLanguages
                    width: parent.width - Theme.windowMargin
                }
            }

        }
    }
}
