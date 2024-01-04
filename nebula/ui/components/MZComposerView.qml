/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
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
                if (modelData instanceof MZComposerBlockButton) repeater.isFirstButton = false
                Layout.topMargin = margin
            }


            function getSourceComponent() {
                if (modelData instanceof MZComposerBlockTitle) return titleBlock
                if (modelData instanceof MZComposerBlockText) return textBlock
                if (modelData instanceof MZComposerBlockOrderedList ||
                    modelData instanceof MZComposerBlockUnorderedList) return listBlock
                if (modelData instanceof MZComposerBlockButton)
                    return composerBlock.style === MZComposerBlockButton.Link ? linkButtonBlock : buttonBlock

                console.error("Unable to create view for composer block of type: " + modelData)
            }

            function getTopMargin() {
                if (modelData instanceof MZComposerBlockTitle) {
                    return MZTheme.theme.vSpacingSmall
                }

                if (modelData instanceof MZComposerBlockText) {
                    switch(composer.view) {
                    case MZComposerView.View.Guide:
                        return MZTheme.theme.listSpacing * 0.5
                    case MZComposerView.View.Message:
                    default:
                        return MZTheme.theme.vSpacingSmall
                    }
                }

                if (modelData instanceof MZComposerBlockOrderedList ||
                        modelData instanceof MZComposerBlockUnorderedList) {
                    switch(composer.view) {
                    case MZComposerView.View.Guide:
                        return MZTheme.theme.listSpacing * 0.5
                    case MZComposerView.View.Message:
                    default:
                        return MZTheme.theme.vSpacingSmall
                    }
                }

                if (modelData instanceof MZComposerBlockButton) {
                    if(repeater.isFirstButton) {
                        return MZTheme.theme.vSpacingSmall * 2
                    }
                    return MZTheme.theme.vSpacingSmall
                }
            }

            Component {
                id: titleBlock

                MZBoldInterLabel {
                    text: loader.composerBlock.title
                    font.pixelSize: MZTheme.theme.fontSize
                    lineHeight: MZTheme.theme.labelLineHeight
                    verticalAlignment: Text.AlignVCenter
                    wrapMode: Text.Wrap
                }
            }

            Component {
                id: textBlock

                MZInterLabel {
                    anchors.left: parent.left
                    anchors.right: parent.right

                    text: loader.composerBlock.text
                    font.pixelSize: MZTheme.theme.fontSizeSmall
                    color: MZTheme.theme.fontColor
                    horizontalAlignment: Text.AlignLeft
                }
            }

            Component {
                id: listBlock

                MZInterLabel {
                    property string listType: loader.composerBlock && composerBlock.type === "olist" ? "ol" : "ul"
                    property var tagsList: loader.composerBlock.subBlocks.map(subBlock => `<li>${subBlock}</li>`)

                    anchors.left: parent.left
                    anchors.right: parent.right

                    text: `<${listType} style='margin-left: -24px;-qt-list-indent:1;'>%1</${listType}>`.arg(tagsList.join(""))
                    textFormat: Text.RichText
                    font.pixelSize: MZTheme.theme.fontSizeSmall
                    color: MZTheme.theme.fontColor
                    horizontalAlignment: Text.AlignLeft
                    Accessible.name: tagsList.join(".\n").replace(repeater.indentTagRegex, "")
                    lineHeight: 20
                }
            }

            Component {
                id: buttonBlock

                MZButton {
                    anchors.left: parent.left
                    anchors.right: parent.right

                    implicitHeight: MZTheme.theme.rowHeight
                    text: loader.composerBlock.text
                    colorScheme: loader.composerBlock.style === MZComposerBlockButton.Primary ? MZTheme.theme.blueButton : MZTheme.theme.redButton

                    onClicked: {
                        Glean.sample.addonCtaClicked.record({
                            addon_id: addon.id
                        });
                        loader.composerBlock.click();
                    }
                }
            }

            Component {
                id: linkButtonBlock

                MZLinkButton {
                    anchors.left: parent.left
                    anchors.right: parent.right

                    labelText: loader.composerBlock.text

                    onClicked: loader.composerBlock.click()
                }
            }
        }
    }
}
