/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1

VPNViewBase {
    property var useSystemLanguageEnabled: toggleCard.toggleChecked

    id: vpnFlickable
    objectName: "settingsLanguagesView"

    //% "Language"
    _menuTitle :  qsTrId("vpn.settings.language")
    _viewContentData: ColumnLayout {
        id: col

        Layout.fillWidth: true
        spacing: VPNTheme.theme.windowMargin

        VPNToggleCard {
            id: toggleCard
            toggleObjectName: "settingsSystemLanguageToggle"
            Layout.fillWidth: true
            Layout.topMargin: -VPNTheme.theme.windowMargin
            Layout.preferredHeight: childrenRect.height

            //% "Use system language"
            //: Title for the language switcher toggle.
            labelText: qsTrId("vpn.settings.systemLanguageTitle")

            //% "Mozilla VPN will use the default system language."
            //: Description for the language switcher toggle when
            //: "Use system language" is enabled.
            sublabelText: qsTrId("vpn.settings.systemLangaugeSubtitle")

            toolTipTitleText: {
                if (toggleChecked) {
                   //% "Disable to select a different language"
                   //: Tooltip for the language switcher toggle
                   return qsTrId("vpn.settings.systemLanguageEnabled");
                }
                return qsTrId("vpn.settings.systemLanguageTitle");
            }

            toggleChecked: VPNLocalizer.code === ""
            function handleClick() {
                toggleChecked = !toggleChecked
                if (toggleChecked) {
                    VPNLocalizer.code = "";
                } else {
                    VPNLocalizer.code = VPNLocalizer.previousCode;
                }
            }
        }

        Column {
            id: languageList
            objectName: "languageList"
            opacity: useSystemLanguageEnabled ? .5 : 1
            spacing: VPNTheme.theme.hSpacing
            Layout.fillWidth: true
            Layout.topMargin: VPNTheme.theme.windowMargin * 1.5
            Layout.leftMargin: VPNTheme.theme.windowMargin * 1.5
            Layout.rightMargin: VPNTheme.theme.windowMargin * 1.5

            VPNSearchBar {
                id: searchBar
                _filterProxySource: VPNLocalizer
                _filterProxyCallback: obj => {
                     const filterValue = getSearchBarText();
                     return obj.localizedLanguage.toLowerCase().includes(filterValue) ||
                             obj.language.toLowerCase().includes(filterValue);
                 }
                _searchBarHasError: repeater.count === 0
                _searchBarPlaceholderText: VPNl18n.LanguageViewSearchPlaceholder

                enabled: !useSystemLanguageEnabled
                anchors.left: parent.left
                anchors.right: parent.right
            }

            Repeater {
                id: repeater

                model: searchBar.getProxyModel()

                Component.onCompleted: {
                    for (let idx = 0; idx < repeater.count; idx++) {
                        const langItem = repeater.itemAt(idx);
                        if (langItem.isSelectedLanguage) {
                            const yCenter = vpnFlickable.height / 2;
                            const selectedItemYPosition = langItem.y + (languageList.y + repeater.y + VPNTheme.theme.menuHeight + toggleCard.height + langItem.height) - yCenter;
                            const destinationY = (selectedItemYPosition + vpnFlickable.height >col.implicitHeight) ? col.implicitHeight - vpnFlickable.height / 2: selectedItemYPosition;
                            if (destinationY < 0) {
                                return;
                            }
                            setContentY(destinationY)
                        }
                    }
                }

                delegate: ColumnLayout {
                    property bool isSelectedLanguage: delRadio.checked
                    id: del
                    spacing: 0
                    objectName: "language-column-" + code

                    function pushFocusToRadio() {
                        delRadio.forceActiveFocus();
                    }

                    VPNRadioDelegate {
                        id: delRadio
                        objectName: "language-" + code
                        enabled: !useSystemLanguageEnabled
                        radioButtonLabelText: localizedLanguage
                        checked: VPNLocalizer.code === code && !useSystemLanguageEnabled
                        onClicked: {
                            VPNLocalizer.code = code;
                        }

                        //% "%1 %2"
                        //: This string is read by accessibility tools.
                        //: %1 is the language name, %2 is the localized language name.
                        accessibleName: qsTrId("vpn.settings.languageAccessibleName")
                                            .arg(language)
                                            .arg(localizedLanguage)

                        activeFocusOnTab: !useSystemLanguageEnabled
                        Keys.onDownPressed: repeater.itemAt(index + 1) ? repeater.itemAt(index + 1).pushFocusToRadio() : repeater.itemAt(0).pushFocusToRadio()
                        Keys.onUpPressed: repeater.itemAt(index - 1) ? repeater.itemAt(index - 1).pushFocusToRadio() : searchBar.forceActiveFocus()
                    }

                    VPNTextBlock {
                        Layout.leftMargin: delRadio.indicator.implicitWidth + VPNTheme.theme.hSpacing - 2
                        Layout.topMargin: 4
                        Layout.fillWidth: true
                        text: language
                    }
                }
            }
        }
    }
    ButtonGroup {
        id: radioButtonGroup
    }
}

