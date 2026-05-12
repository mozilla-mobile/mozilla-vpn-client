/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import Mozilla.Shared 1.0
import components 0.1

ColumnLayout {
    id: root

    property int dividerSpacing: MZTheme.theme.toggleRowDividerSpacing


    spacing: dividerSpacing
    MZToggleRow {
            objectName: "alwaysUsePort53"

            labelText: MZI18n.SettingsAntiCensorshipPort53Title
            subLabelText: MZI18n.SettingsAntiCensorshipPort53Body
            checked: MZFeatureList.get("alwaysPort53").isSupported
            dividerTopMargin: dividerSpacing
            onClicked: {
                MZFeatureList.toggle("alwaysPort53");
            }
    }

    Repeater {
        id: repeater

        model: [
            {
                objectName: "obfuscationUdpOverTcp",
                settingValue: MZSettings.UdpOverTcp,
                settingTitle: MZI18n.SettingsAntiCensorshipUdpOverTcpTitle,
                settingDescription: MZI18n.SettingsAntiCensorshipUdpOverTcpBody,
            }, {
                objectName: "obfuscationLwo",
                settingValue: MZSettings.LWO,
                settingTitle: MZI18n.SettingsAntiCensorshipLwoTitle,
                settingDescription: MZI18n.SettingsAntiCensorshipLwoBody,
            }, {
                objectName: "obfuscationMasque",
                settingValue: MZSettings.Masque,
                settingTitle: MZI18n.SettingsAntiCensorshipMasqueTitle,
                settingDescription: MZI18n.SettingsAntiCensorshipMasqueBody,
            },{
                objectName: "obfuscationShadowsocks",
                settingValue: MZSettings.Shadowsocks,
                settingTitle: MZI18n.SettingsAntiCensorshipShadowsocksTitle,
                settingDescription: MZI18n.SettingsAntiCensorshipShadowsocksBody,
            }
        ];

        delegate: MZToggleRow {
            objectName: modelData.objectName

            labelText: modelData.settingTitle
            subLabelText: modelData.settingDescription
            checked: MZSettings.obfuscationMethod == modelData.settingValue
            dividerTopMargin: dividerSpacing
            visible: MZFeatureList.get(modelData.objectName).isSupported
            showDivider: !isOnboarding || index + 1 !== repeater.model.length

            onClicked: {
                if(MZSettings.obfuscationMethod == modelData.settingValue) {
                    MZSettings.obfuscationMethod = MZSettings.NoObfuscation;
                } else {
                    MZSettings.obfuscationMethod = modelData.settingValue;
                    // Maybe automatically disable alwaysPort53 when enabling an obfuscation method 
                    // that requires a specific port. Right now the port it's overridden in the controller.
                }
            }
        }
    }
}
