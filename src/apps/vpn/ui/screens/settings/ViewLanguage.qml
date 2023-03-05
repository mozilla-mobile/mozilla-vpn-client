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
    property var useSystemLanguageEnabled: toggleCard.toggleChecked

    id: vpnFlickable
    objectName: "settingsLanguagesView"

    //% "Language"
    _menuTitle :  qsTrId("vpn.settings.language")
    _viewContentData: ColumnLayout {
        id: col

        Layout.fillWidth: true
        spacing: MZTheme.theme.windowMargin

        MZToggleCard {
            id: toggleCard
            toggleObjectName: "settingsSystemLanguageToggle"
            Layout.fillWidth: true
            Layout.topMargin: -MZTheme.theme.windowMargin
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

            toggleChecked: MZSettings.languageCode === ""
            function handleClick() {
                toggleChecked = !toggleChecked
                if (toggleChecked) {
                    MZSettings.languageCode = "";
                } else {
                    MZSettings.languageCode = MZSettings.previousLanguageCode;
                }
            }
        }

        Column {
            id: languageList
            objectName: "languageList"
            opacity: useSystemLanguageEnabled ? .5 : 1
            spacing: MZTheme.theme.hSpacing
            Layout.fillWidth: true
            Layout.topMargin: MZTheme.theme.windowMargin * 1.5
            Layout.leftMargin: MZTheme.theme.windowMargin * 1.5
            Layout.rightMargin: MZTheme.theme.windowMargin * 1.5

            MZSearchBar {
                id: searchBar
                _filterProxySource: MZLocalizer
                _filterProxyCallback: obj => {
                     const filterValue = getSearchBarText();
                     return obj.nativeLanguageName.toLowerCase().includes(filterValue) ||
                             obj.localizedLanguageName.toLowerCase().includes(filterValue);
                 }
                _searchBarHasError: repeater.count === 0
                _searchBarPlaceholderText: MZI18n.LanguageViewSearchPlaceholder

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
                    property bool isSelectedLanguage: delRadio.checked
                    id: del
                    spacing: 0
                    objectName: "language-column-" + code
                    width: parent.width

                    LayoutMirroring.enabled: isRightToLeft
                    LayoutMirroring.childrenInherit: true

                    function pushFocusToRadio() {
                        delRadio.forceActiveFocus();
                    }

                    MZRadioDelegate {
                        id: delRadio
                        objectName: "language-" + code
                        enabled: !useSystemLanguageEnabled
                        radioButtonLabelText: nativeLanguageName
                        checked: MZSettings.languageCode === code && !useSystemLanguageEnabled
                        onClicked: {
                            MZSettings.languageCode = code;
                        }

                        Layout.alignment: Qt.AlignLeft

                        //% "%1 %2"
                        //: This string is read by accessibility tools.
                        //: %1 is the language name, %2 is the localized language name.
                        accessibleName: qsTrId("vpn.settings.languageAccessibleName")
                                            .arg(nativeLanguageName)
                                            .arg(localizedLanguageName)

                        activeFocusOnTab: !useSystemLanguageEnabled
                        Keys.onDownPressed: repeater.itemAt(index + 1) ? repeater.itemAt(index + 1).pushFocusToRadio() : repeater.itemAt(0).pushFocusToRadio()
                        Keys.onUpPressed: repeater.itemAt(index - 1) ? repeater.itemAt(index - 1).pushFocusToRadio() : searchBar.forceActiveFocus()
                    }

                    MZTextBlock {
                        Layout.leftMargin: delRadio.indicator.implicitWidth + MZTheme.theme.hSpacing - 2
                        Layout.topMargin: 4
                        Layout.fillWidth: true
                        text: localizedLanguageName
                        Layout.alignment: Qt.AlignLeft
                    }
                }
            }
        }
    }
    ButtonGroup {
        id: radioButtonGroup
    }
}

