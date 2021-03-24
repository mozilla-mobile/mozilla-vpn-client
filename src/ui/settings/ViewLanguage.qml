/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import Mozilla.VPN 1.0
import "../components"
import "../themes/themes.js" as Theme

Item {
    id: container

    readonly property int defaultMargin: 18

    // TODO: remove when re-landing the system-language
    //% "Mozilla VPN will use your system’s default language."
    //: Description for the language switcher toggle when
    //: "Use system language" is enabled.
    property var systemLanguageEnabledSubtitle: qsTrId("vpn.settings.systemLangaugeEnabledSubtitle")

    // TODO: remove when re-landing the system-language
    //% "Mozilla VPN will not use the default system language."
    //: Description for the language switcher toggle when
    //: "Use system language" is disabled.
    property var systemLanguageDisabledSubtitle: qsTrId("vpn.settings.systemLanguageDisabledSubtitle")

    // TODO: remove when re-landing the system-language
    //% "Use system language"
    //: Title for the language switcher toggle.
    property var systemLanguageTitle: qsTrId("vpn.settings.systemLanguageTitle")

    // TODO: remove when re-landing the system-language
    //% "Disable to select a different language"
    //: Tooltip for the language switcher toggle
    property var systemLanguageEnabled: qsTrId("vpn.settings.systemLanguageEnabled")

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

        property var lastFocusedItemIdx

        height: parent.height - menu.height
        anchors.top: menu.bottom
        width: parent.width
        onActiveFocusChanged: if (focus && lastFocusedItemIdx) repeater.itemAt(lastFocusedItemIdx).forceActiveFocus()
        Accessible.name: menu.title
        Accessible.role: Accessible.List

        ButtonGroup {
            id: radioButtonGroup
        }

        VPNFlickable {
            id: vpnFlickable

            objectName: "settingsLanguagesView"
            flickContentHeight: col.y + col.implicitHeight + (Theme.rowHeight * 2)
            anchors.fill: parent

            NumberAnimation on contentY {
                id: scrollAnimation

                duration: 200
                easing.type: Easing.OutQuad
            }

            Rectangle {
                id: verticalSpacer

                height: Theme.windowMargin
                width: parent.width
                color: "transparent"
            }

            Column {
                id: col

                objectName: "languageList"

                spacing: 20
                width: parent.width
                anchors.top: verticalSpacer.bottom
                Component.onCompleted: {

                    // Scroll vpnFlickable so that the current language is
                    // vertically centered in the view

                    const yCenter = vpnFlickable.height / 2;

                    for (let idx = 0; idx < repeater.count; idx++) {
                        const repeaterItem = repeater.itemAt(idx);
                        const repeaterItemYPosition = repeaterItem.mapToItem(vpnFlickable.contentItem, 0, 0).y;
                        if (!repeaterItem.checked || repeaterItemYPosition < yCenter) {
                            continue;
                        }

                        const selectedItemYPosition = repeaterItem.y + (Theme.rowHeight * 1.5) - yCenter;
                        const destinationY = (selectedItemYPosition + vpnFlickable.height > vpnFlickable.contentHeight) ? vpnFlickable.contentHeight - vpnFlickable.height : selectedItemYPosition;

                        vpnFlickable.contentY = destinationY;
                        return;
                    }
                }

                function scrollDelegateIntoView(item) {
                    if (window.height > vpnFlickable.contentHeight) {
                        return;
                    }
                    const yPosition = item.mapToItem(vpnFlickable.contentItem, 0, 0).y;
                    const approximateDelegateHeight = 60;
                    const ext = approximateDelegateHeight + yPosition;

                    if (yPosition < vpnFlickable.contentY || yPosition > vpnFlickable.contentY + vpnFlickable.height || ext < vpnFlickable.contentY || ext > vpnFlickable.contentY + vpnFlickable.height) {
                        const destinationY = Math.max(0, Math.min(yPosition - vpnFlickable.height + approximateDelegateHeight, vpnFlickable.contentHeight - vpnFlickable.height));
                        scrollAnimation.to = destinationY;
                        scrollAnimation.start();
                    }
                }

                Repeater {
                    id: repeater

                    model: VPNLocalizer
                    delegate: VPNRadioDelegate {
                        property bool isSelectedLanguage: checked

                        id: del
                        objectName: "language-" + code

                        radioButtonLabelText: localizedLanguage
                        checked: VPNLocalizer.code === code
                        onClicked: VPNLocalizer.code = code
                        anchors.left: parent.left
                        anchors.leftMargin: defaultMargin
                        width: parent.width - defaultMargin * 2
                        //% "%1 %2"
                        //: This string is read by accessibility tools.
                        //: %1 is the language name, %2 is the localized language name.
                        accessibleName: qsTrId("vpn.settings.languageAccessibleName")
                            .arg(language)
                            .arg(localizedLanguage)

                        activeFocusOnTab: true
                        onActiveFocusChanged: col.scrollDelegateIntoView(del)
                        Keys.onDownPressed: repeater.itemAt(index + 1) ? repeater.itemAt(index + 1).forceActiveFocus() : repeater.itemAt(0).forceActiveFocus()
                        Keys.onUpPressed: repeater.itemAt(index - 1) ? repeater.itemAt(index - 1).forceActiveFocus() : menu.forceActiveFocus()
                        Keys.onBacktabPressed: {
                            focusScope.lastFocusedItemIdx = index;
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
