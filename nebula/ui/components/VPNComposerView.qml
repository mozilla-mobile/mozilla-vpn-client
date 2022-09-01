/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Layouts 1.15
import Qt.labs.qmlmodels

import Mozilla.VPN 1.0
import Mozilla.VPN.qmlcomponents 1.0
import components 0.1

ColumnLayout {
    id: composer
    property var addon
    required property int view

    enum View {
        Guide = 0,
        Message = 1
    }

    spacing: 0

    Repeater {
        id: repeater
        property var indentTagRegex: /(<\/?li>)/g
        property bool isFirstButton: true

        model: addon.composer.blocks
        delegate: Loader {
            id: loader

            property var composerBlock: modelData

            Layout.fillWidth: true
            Layout.preferredHeight: item.implicitHeight

            sourceComponent: getSourceComponent()

            Component.onCompleted: {
                //Avoids binding loops
                var margin = getTopMargin()
                if (modelData instanceof VPNComposerBlockButton) repeater.isFirstButton = false
                Layout.topMargin = margin
            }


            function getSourceComponent() {
                if (modelData instanceof VPNComposerBlockTitle) return titleBlock
                if (modelData instanceof VPNComposerBlockText) return textBlock
                if (modelData instanceof VPNComposerBlockOrderedList ||
                    modelData instanceof VPNComposerBlockUnorderedList) return listBlock
                if (modelData instanceof VPNComposerBlockButton)
                    return composerBlock.style === ComposerBlockButtonStyle.Link ? linkButtonBlock : buttonBlock
            }

            function getTopMargin() {
                if (modelData instanceof VPNComposerBlockTitle) {
                    switch(composer.view) {
                    case VPNComposerView.View.Guide:
                    case VPNComposerView.View.Message:
                    default:
                        return VPNTheme.theme.vSpacingSmall
                    }
                }

                if (modelData instanceof VPNComposerBlockText) {
                    switch(composer.view) {
                    case VPNComposerView.View.Guide:
                        return VPNTheme.theme.listSpacing * 0.5
                    case VPNComposerView.View.Message:
                    default:
                        return VPNTheme.theme.vSpacingSmall
                    }
                }

                if (modelData instanceof VPNComposerBlockOrderedList ||
                        modelData instanceof VPNComposerBlockUnorderedList) {
                    switch(composer.view) {
                    case VPNComposerView.View.Guide:
                        return VPNTheme.theme.listSpacing * 0.5
                    case VPNComposerView.View.Message:
                    default:
                        return VPNTheme.theme.vSpacingSmall
                    }
                }

                if (modelData instanceof VPNComposerBlockButton) {
                    switch(composer.view) {
                    case VPNComposerView.View.Guide:
                    case VPNComposerView.View.Message:
                    default:
                        if(repeater.isFirstButton) {
                            return VPNTheme.theme.vSpacingSmall * 2
                        }

                        return VPNTheme.theme.vSpacingSmall
                    }
                }
            }

            Component {
                id: titleBlock

                VPNBoldInterLabel {
                    text: loader.composerBlock.title
                    font.pixelSize: VPNTheme.theme.fontSize
                    lineHeight: VPNTheme.theme.labelLineHeight
                    verticalAlignment: Text.AlignVCenter
                    wrapMode: Text.Wrap
                }
            }

            Component {
                id: textBlock

                VPNInterLabel {
                    text: loader.composerBlock.text
                    font.pixelSize: VPNTheme.theme.fontSizeSmall
                    color: VPNTheme.theme.fontColor
                    horizontalAlignment: Text.AlignLeft
                }
            }

            Component {
                id: listBlock

                VPNInterLabel {
                    property string listType: loader.composerBlock && composerBlock.type === "olist" ? "ol" : "ul"
                    property var tagsList: loader.composerBlock.subBlocks.map(subBlock => `<li>${subBlock}</li>`)

                    text: `<${listType} style='margin-left: -24px;-qt-list-indent:1;'>%1</${listType}>`.arg(tagsList.join(""))
                    textFormat: Text.RichText
                    font.pixelSize: VPNTheme.theme.fontSizeSmall
                    color: VPNTheme.theme.fontColor
                    horizontalAlignment: Text.AlignLeft
                    Accessible.name: tagsList.join("\n").replace(repeater.indentTagRegex, "")
                    lineHeight: 20
                }
            }

            Component {
                id: buttonBlock

                VPNButton {
                    implicitHeight: VPNTheme.theme.rowHeight
                    text: loader.composerBlock.text
                    colorScheme: loader.composerBlock.style === ComposerBlockButtonStyle.Primary ? VPNTheme.theme.blueButton : VPNTheme.theme.redButton
                }
            }

            Component {
                id: linkButtonBlock

                VPNLinkButton {
                    labelText: loader.composerBlock.text
                }
            }
        }
    }
}
