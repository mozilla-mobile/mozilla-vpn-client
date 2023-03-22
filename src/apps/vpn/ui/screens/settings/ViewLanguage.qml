/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import components 0.1
import components.forms 0.1

MZViewBase {
    id: vpnFlickable
    objectName: "settingsLanguagesView"

    //% "Language"
    _menuTitle :  qsTrId("vpn.settings.language")
    _viewContentData: ColumnLayout {
        id: col
        objectName: "languageList"

        Layout.fillWidth: true

        spacing: MZTheme.theme.vSpacing

        ButtonGroup {
            id: radioButtonGroup
        }

        MZSearchBar {
            id: searchBar

            Layout.fillWidth: true
            Layout.leftMargin: MZTheme.theme.windowMargin * 1.5
            Layout.rightMargin: MZTheme.theme.windowMargin * 1.5

            _filterProxySource: MZLocalizer
            _filterProxyCallback: obj => {
                                      const filterValue = getSearchBarText();
                                      return obj.nativeLanguageName.toLowerCase().includes(filterValue) ||
                                      obj.localizedLanguageName.toLowerCase().includes(filterValue);
                                  }
            _searchBarHasError: repeater.count === 0
            _searchBarPlaceholderText: MZI18n.LanguageViewSearchPlaceholder

        }

        ColumnLayout {
            spacing: 0
            visible: searchBar.isEmpty

            MZRadioDelegate {
                id: systemLanguageRadioButton

                Layout.fillWidth: true
                Layout.leftMargin: MZTheme.theme.windowMargin
                Layout.rightMargin: MZTheme.theme.windowMargin

                radioButtonLabelText: MZI18n.LanguageViewSystemLanguageButtonTitle
                checked: MZSettings.languageCode === ""
                activeFocusOnTab: true
                onClicked: {
                    MZSettings.languageCode = ""
                }

                Keys.onDownPressed: if(repeater.itemAt(0)) repeater.itemAt(0).pushFocusToRadio()

                accessibleName: `${radioButtonLabelText}. ${systemLanguageRadioButtonDescription.text}`
            }

            MZTextBlock {
                id: systemLanguageRadioButtonDescription

                Layout.fillWidth: true
                Layout.topMargin: 4
                Layout.leftMargin: MZTheme.theme.windowMargin + systemLanguageRadioButton.labelX
                Layout.rightMargin: MZTheme.theme.windowMargin

                text: qsTrId("vpn.settings.systemLangaugeSubtitle")
            }

            Rectangle {                
                Layout.preferredHeight: 1
                Layout.fillWidth: true
                Layout.topMargin: MZTheme.theme.vSpacing
                Layout.leftMargin: 18.5
                Layout.rightMargin: 18.5

                color: "#E7E7E7"
            }
        }

        Repeater {
            id: repeater

            model: searchBar.getProxyModel()

            Component.onCompleted: {
                for (let idx = 0; idx < repeater.count; idx++) {
                    const langItem = repeater.itemAt(idx);
                    if (langItem.isSelectedLanguage) {
                        const yCenter = vpnFlickable.height / 2;
                        const selectedItemYPosition = langItem.y + (languageList.y + repeater.y + MZTheme.theme.menuHeight + toggleCard.height + langItem.height) - yCenter;
                        const destinationY = (selectedItemYPosition + vpnFlickable.height >col.implicitHeight) ? col.implicitHeight - vpnFlickable.height / 2: selectedItemYPosition;
                        if (destinationY < 0) {
                            return;
                        }
                        setContentY(destinationY)
                    }
                }
            }

            delegate: ColumnLayout {
                id: del
                objectName: "language-column-" + code

                property bool isSelectedLanguage: delRadio.checked

                function pushFocusToRadio() {
                    delRadio.forceActiveFocus();
                }

                spacing: 0
                width: parent.width

                LayoutMirroring.enabled: isRightToLeft
                LayoutMirroring.childrenInherit: true

                MZRadioDelegate {
                    id: delRadio
                    objectName: "language-" + code

                    Layout.fillWidth: true
                    Layout.leftMargin: MZTheme.theme.windowMargin
                    Layout.rightMargin: MZTheme.theme.windowMargin

                    radioButtonLabelText: nativeLanguageName
                    checked: MZSettings.languageCode === code
                    activeFocusOnTab: true
                    onClicked: {
                        MZSettings.languageCode = code;
                    }

                    Keys.onDownPressed: if(repeater.itemAt(index + 1)) repeater.itemAt(index + 1).pushFocusToRadio()
                    Keys.onUpPressed: {
                        if(repeater.itemAt(index - 1)) repeater.itemAt(index - 1).pushFocusToRadio()
                        else systemLanguageRadioButton.forceActiveFocus()
                    }

                    //% "%1 %2"
                    //: This string is read by accessibility tools.
                    //: %1 is the language name, %2 is the localized language name.
                    accessibleName: qsTrId("vpn.settings.languageAccessibleName")
                    .arg(nativeLanguageName)
                    .arg(localizedLanguageName)
                }

                MZTextBlock {
                    Layout.leftMargin: MZTheme.theme.windowMargin + delRadio.labelX
                    Layout.topMargin: 4
                    Layout.rightMargin: MZTheme.theme.windowMargin
                    Layout.fillWidth: true

                    text: localizedLanguageName
                }
            }
        }
    }
}

