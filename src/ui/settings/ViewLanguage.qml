/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11
import Mozilla.VPN 1.0
import "../components"
import "../themes/themes.js" as Theme

Item {
    id: container

    readonly property int defaultMargin: 18
    property var useSystemLanguageEnabled: useSystemLanguageToggle.checked

    VPNMenu {
        id: menu
        objectName: "settingsLanguagesBackButton"

        //% "Language"
        title: qsTrId("vpn.settings.language")
        isSettingsView: true
        onActiveFocusChanged: if (focus) forceFocus = true
    }

    FocusScope {
        id: focusScope

        property var lastFocusedItemIdx: -1

        height: parent.height - menu.height
        anchors.top: menu.bottom
        width: parent.width
        onActiveFocusChanged: {
            if (focus && lastFocusedItemIdx !== -1) {
              repeater.itemAt(lastFocusedItemIdx).forceActiveFocus();
          } else if (focus) {
                useSystemLanguageToggle.forceActiveFocus();
            }
        }
        Accessible.name: menu.title
        Accessible.role: Accessible.List

        ButtonGroup {
            id: radioButtonGroup
        }


        VPNFlickable {
            id: vpnFlickable

            objectName: "settingsLanguagesView"
            flickContentHeight: row.y + row.implicitHeight + col.y + col.implicitHeight + (Theme.rowHeight * 2)
            anchors.fill: parent

            RowLayout {
                id: row
                width: parent.width - (defaultMargin * 2)
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: defaultMargin
                anchors.rightMargin: defaultMargin
                anchors.top: parent.top
                anchors.topMargin: 20
                spacing: 12

                ColumnLayout {
                    id: labelWrapper
                    spacing: 4
                    Layout.maximumWidth: parent.width - useSystemLanguageToggle.width - 16

                    VPNInterLabel {
                        id: label
                        Layout.alignment: Qt.AlignLeft
                        //% "Use system language"
                        //: Title for the language switcher toggle.
                        text: qsTrId("vpn.settings.systemLanguageTitle")
                        color: Theme.fontColorDark
                        horizontalAlignment: Text.AlignLeft
                        Layout.fillWidth: true
                    }

                    VPNTextBlock {
                        id: labelDescription
                        Layout.fillWidth: true
                        //% "Mozilla VPN will use the default system language."
                        //: Description for the language switcher toggle when
                        //: "Use system language" is enabled.
                        text: qsTrId("vpn.settings.systemLangaugeSubtitle")
                    }
                }

                VPNSettingsToggle {
                    id: useSystemLanguageToggle

                    objectName: "settingsSystemLanguageToggle"
                    toolTipTitle: {
                        if (checked) {
                           //% "Disable to select a different language"
                           //: Tooltip for the language switcher toggle
                           return qsTrId("vpn.settings.systemLanguageEnabled");
                        }
                        return qsTrId("vpn.settings.systemLanguageTitle");
                    }
                    onActiveFocusChanged: {
                        if (focus) {
                            forceFocus = true;
                            focusScope.lastFocusedItemIdx = -1;
                            vpnFlickable.ensureVisible(useSystemLanguageToggle);
                      }
                    }
                    Layout.preferredHeight: 24
                    Layout.preferredWidth: 45
                    width: undefined
                    height: undefined
                    Keys.onDownPressed: if (!checked) repeater.itemAt(0).forceActiveFocus()
                    checked: VPNLocalizer.code === ""
                    onClicked: {
                        checked = !checked;
                        if (checked) {
                            VPNLocalizer.code = "";
                        } else {
                            VPNLocalizer.code = VPNLocalizer.previousCode;
                        }
                    }
                }
            }

            Rectangle {
                id: divider
                height: 1
                width: parent.width - 36
                anchors.top: row.bottom
                anchors.topMargin: defaultMargin
                anchors.horizontalCenter: parent.horizontalCenter
                color: "#E7E7E7"
                opacity: 1
            }

            Column {
                id: col

                objectName: "languageList"

                spacing: 20
                width: parent.width
                anchors.top: divider.bottom
                anchors.topMargin: 20
                Component.onCompleted: {

                    if (useSystemLanguageEnabled) {
                        return;
                    }

                    // Scroll vpnFlickable so that the current language is
                    // vertically centered in the view

                    const yCenter = (vpnFlickable.height - menu.height ) / 2

                    for (let idx = 0; idx < repeater.count; idx++) {
                        const repeaterItem = repeater.itemAt(idx);
                        const repeaterItemYPosition = repeaterItem.mapToItem(vpnFlickable.contentItem, 0, 0).y;
                        if (!repeaterItem.checked || repeaterItemYPosition < yCenter) {
                            continue;
                        }

                        const selectedItemYPosition = repeaterItem.y + (Theme.rowHeight * 3) - yCenter;
                        const destinationY = (selectedItemYPosition + vpnFlickable.height > vpnFlickable.contentHeight) ? vpnFlickable.contentHeight - vpnFlickable.height : selectedItemYPosition;

                        // Prevent edge case negative scrolling
                        if (destinationY < 0) {
                            return;
                        }

                        vpnFlickable.contentY = destinationY;
                        return;
                    }
                }

                Repeater {
                    id: repeater

                    model: VPNLocalizer
                    delegate: VPNRadioDelegate {
                        property bool isSelectedLanguage: checked

                        id: del
                        objectName: "language-" + code
                        enabled: !useSystemLanguageEnabled

                        opacity: useSystemLanguageEnabled ? .5 : 1
                        radioButtonLabelText: localizedLanguage
                        checked: VPNLocalizer.code === code && !useSystemLanguageEnabled
                        onClicked: {
                            VPNLocalizer.code = code;
                        }
                        anchors.left: parent.left
                        anchors.leftMargin: defaultMargin
                        width: parent.width - defaultMargin * 2
                        //% "%1 %2"
                        //: This string is read by accessibility tools.
                        //: %1 is the language name, %2 is the localized language name.
                        accessibleName: qsTrId("vpn.settings.languageAccessibleName")
                            .arg(language)
                            .arg(localizedLanguage)

                        activeFocusOnTab: !useSystemLanguageEnabled
                        onActiveFocusChanged: {
                            if (focus) {
                                vpnFlickable.ensureVisible(del);
                                focusScope.lastFocusedItemIdx = index;
                            }
                        }
                        Keys.onDownPressed: repeater.itemAt(index + 1) ? repeater.itemAt(index + 1).forceActiveFocus() : repeater.itemAt(0).forceActiveFocus()
                        Keys.onUpPressed: repeater.itemAt(index - 1) ? repeater.itemAt(index - 1).forceActiveFocus() : useSystemLanguageToggle.forceActiveFocus()
                        Keys.onBacktabPressed: {
                            if (index === 0) {
                                useSystemLanguageToggle.forceActiveFocus();
                                return;
                            }
                            menu.forceActiveFocus();
                        }

                        VPNRadioSublabel {
                            text: language
                        }

                    }
                }
            }
        }
    }

}
