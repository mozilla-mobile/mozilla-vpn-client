/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import components 0.1
import Mozilla.Shared 1.0

//SUMMARY: MZHelpSheet is a component derived from MZBottomSheet that opens a drawer from the bottom of the screen who's content
//is always laid out in a consistnet way by passing in a model with the correct schema, and sizes to fit the content it contains

//USAGE: pass a list of blocks into the model to instantiate components as follows:

//Fields:
//"type": MZHelpSheet.BlockType (eg MZHelpSheet.BlockType.PrimaryButton)
//"text": label or button text (eg "Hello world")
//"margin": top margin above the component (eg 16)
//"action": action performed on button click - only applicable when type is MZHelpSheet.BlockType.PrimaryButton or MZHelpSheet.BlockType.LinkButton
//(eg () => { MZUrlOpener.openUrl("https://mozilla.org") })
/*
  MZHelpSheet {
      title: "MZHelpSheet"

      model: [
          {type: MZHelpSheet.BlockType.Title, text: "title"},
          {type: MZHelpSheet.BlockType.Text, margin: 8, text: "text"},
          {type: MZHelpSheet.BlockType.Text, margin: 16, text: "text"},
          {type: MZHelpSheet.BlockType.Text, margin: 16, text: "text"},
          {type: MZHelpSheet.BlockType.PrimaryButton, text: "Primary", margin: 16, action: () => { sheet.close(); MZNavigator.requestScreen(VPN.ScreenGetHelp) } },
          {type: MZHelpSheet.BlockType.LinkButton, text: "Link", margin: 8, action: () => { MZUrlOpener.openUrl("https://mozilla.org") } }
      ]
    }
*/

MZBottomSheet {
    id: bottomSheet

    property string title
    required property var model

    enum BlockType {
        Title,
        Text,
        PrimaryButton,
        LinkButton
    }

    sizeToContent: true

    //Only load the content if the drawer itself is loaded
    contentItem: Loader {

        active: bottomSheet.active
        sourceComponent: ColumnLayout {

            spacing: 0

            Accessible.role: Accessible.Grouping
            Accessible.name: bottomSheet.title

            Component.onCompleted: forceActiveFocus()

            ColumnLayout {
                id: headerLayout

                Layout.topMargin: 8
                Layout.preferredWidth: parent.width

                spacing: 0

                RowLayout {
                    spacing: 0

                    Item {
                        Layout.topMargin: MZTheme.theme.windowMargin / 2
                        Layout.leftMargin: MZTheme.theme.windowMargin
                        Layout.preferredHeight: MZTheme.theme.iconSize * 1.5
                        Layout.preferredWidth: MZTheme.theme.iconSize * 1.5
                        Layout.alignment: Qt.AlignTop

                        Image {
                            anchors.centerIn: parent

                            source: "qrc:/nebula/resources/tip-filled.svg"
                            sourceSize.width: MZTheme.theme.iconSize * 1.5

                            mirror: MZLocalizer.isRightToLeft
                            fillMode: Image.PreserveAspectFit
                        }
                    }


                    MZBoldLabel {
                        id: title

                        Layout.topMargin: MZTheme.theme.windowMargin / 2
                        Layout.leftMargin: 8
                        Layout.alignment: Qt.AlignTop
                        Layout.fillWidth: true

                        text: bottomSheet.title
                        verticalAlignment: Text.AlignVCenter
                        lineHeightMode: Text.FixedHeight
                        lineHeight: 24
                        elide: Text.ElideRight
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    MZIconButton {
                        objectName: bottomSheet.objectName + "-closeButton"

                        Layout.rightMargin: MZTheme.theme.windowMargin / 2

                        Layout.preferredHeight: MZTheme.theme.rowHeight
                        Layout.preferredWidth: MZTheme.theme.rowHeight

                        onClicked: bottomSheet.close()

                        accessibleName: MZI18n.GlobalClose

                        Image {
                            anchors.centerIn: parent

                            sourceSize.height: MZTheme.theme.iconSize
                            sourceSize.width: MZTheme.theme.iconSize

                            source: "qrc:/nebula/resources/close-dark.svg"
                            fillMode: Image.PreserveAspectFit
                        }
                    }
                }

                Rectangle {
                    Layout.topMargin: 8
                    Layout.preferredHeight: MZTheme.theme.dividerHeight
                    Layout.fillWidth: true

                    color: MZTheme.colors.grey10
                }
            }

            MZFlickable {
                id: flickable

                readonly property int maxheight: bottomSheet.maxSheetHeight - headerLayout.implicitHeight - headerLayout.Layout.topMargin

                Layout.fillWidth: true
                Layout.preferredHeight: Math.min(layout.implicitHeight + layout.anchors.topMargin + layout.anchors.bottomMargin, maxheight)

                flickContentHeight: layout.implicitHeight + layout.anchors.topMargin + layout.anchors.bottomMargin

                addNavbarHeightOffset: false

                ColumnLayout {
                    id: layout

                    anchors.fill: parent
                    anchors.margins: MZTheme.theme.windowMargin * 1.5

                    Repeater {
                        model: bottomSheet.model
                        delegate: Loader {
                            id: loader
                            objectName: "helpSheetContentLoader"

                            property var composerBlock: bottomSheet.model[index]

                            function getSourceComponent() {
                                switch (composerBlock.type) {
                                case MZHelpSheet.BlockType.Title:
                                    return titleBlock
                                case MZHelpSheet.BlockType.Text:
                                    return textBlock
                                case MZHelpSheet.BlockType.PrimaryButton:
                                    return buttonBlock
                                case MZHelpSheet.BlockType.LinkButton:
                                    return linkButtonBlock
                                default:
                                    return console.error("Unable to create view for composer block of type: " + modelData)
                                }

                            }

                            Layout.fillWidth: true
                            Layout.preferredHeight: item.implicitHeight
                            Layout.topMargin: composerBlock.margin

                            sourceComponent: getSourceComponent()

                            Component {
                                id: titleBlock

                                MZBoldInterLabel {
                                    Layout.fillWidth: true

                                    text: loader.composerBlock.text
                                    font.pixelSize: MZTheme.theme.fontSize
                                    lineHeight: 24
                                    verticalAlignment: Text.AlignVCenter
                                }
                            }

                            Component {
                                id: textBlock

                                MZInterLabel {

                                    text: loader.composerBlock.text
                                    font.pixelSize: MZTheme.theme.fontSizeSmall
                                    color: MZTheme.theme.fontColor
                                    lineHeight: 21
                                    horizontalAlignment: Text.AlignLeft
                                }
                            }

                            Component {
                                id: buttonBlock

                                MZButton {
                                    anchors.left: parent.left
                                    anchors.right: parent.right

                                    implicitHeight: MZTheme.theme.rowHeight

                                    text: loader.composerBlock.text

                                    onClicked: loader.composerBlock.action()
                                }
                            }

                            Component {
                                id: linkButtonBlock

                                MZLinkButton {
                                    anchors.left: parent.left
                                    anchors.right: parent.right

                                    labelText: loader.composerBlock.text

                                    onClicked: loader.composerBlock.action()
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
