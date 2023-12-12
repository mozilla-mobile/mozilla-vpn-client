import QtQuick 2.15
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import components 0.1
import Mozilla.Shared 1.0

//Usage: pass a list of blocks into the model
/*
  MZHelpSheet {
      title: "MZHelpSheet"
      iconSource: "qrc:/ui/resources/connection-info-dark.svg"

      model: [
          {type: MZHelpSheet.BlockType.Title, text: "title"},
          {type: MZHelpSheet.BlockType.Text, text: "text"},
          {type: MZHelpSheet.BlockType.Text, text: "text"},
          {type: MZHelpSheet.BlockType.Text, text: "text"},
          {type: MZHelpSheet.BlockType.PrimaryButton, text: "Primary", action: () => { sheet.close(); MZNavigator.requestScreen(VPN.ScreenGetHelp) } },
          {type: MZHelpSheet.BlockType.LinkButton, text: "Link", action: () => { MZUrlOpener.openUrl("https://mozilla.org") } }
      ]
    }
*/

MZBottomSheet {
    id: bottomSheet

    property alias iconSource: icon.source
    property alias title: title.text
    property var model


    enum BlockType {
        Title,
        Text,
        PrimaryButton,
        LinkButton
    }

    implicitHeight: Math.min(contentItem.implicitHeight, maxHeight)

    contentItem: ColumnLayout {
        spacing: 0

        ColumnLayout {
            id: headerLayout

            Layout.topMargin: 8

            spacing: 0

            RowLayout {
                spacing: 0

                Item {
                    Layout.topMargin: MZTheme.theme.windowMargin / 2
                    Layout.leftMargin: MZTheme.theme.windowMargin
                    Layout.preferredHeight: 24
                    Layout.preferredWidth: 24
                    Layout.alignment: Qt.AlignTop

                    Image {
                        id: icon
                        anchors.centerIn: parent

                        sourceSize.width: MZTheme.theme.iconSize

                        mirror: MZLocalizer.isRightToLeft
                        fillMode: Image.PreserveAspectFit
                    }
                }


                MZBoldLabel {
                    id: title

                    Layout.topMargin: MZTheme.theme.windowMargin / 2
                    Layout.leftMargin: 8
                    Layout.alignment: Qt.AlignTop

                    verticalAlignment: Text.AlignVCenter
                    lineHeightMode: Text.FixedHeight
                    lineHeight: 24
                }

                Item {
                    Layout.fillWidth: true
                }

                MZIconButton {
                    id: iconButton

                    Layout.rightMargin: MZTheme.theme.windowMargin / 2

                    Layout.preferredHeight: MZTheme.theme.rowHeight
                    Layout.preferredWidth: MZTheme.theme.rowHeight

                    //                skipEnsureVisible: true // prevents scrolling of lists when this is focused

                    onClicked: bottomSheet.close()

                    accessibleName: MZI18n.GlobalClose
                    Accessible.ignored: !visible

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
                Layout.preferredHeight: 1
                Layout.preferredWidth: parent.width

                color: MZTheme.colors.grey10
            }
        }


        MZFlickable {
            id: flickable

            readonly property int maxheight: bottomSheet.maxHeight - headerLayout.implicitHeight - headerLayout.Layout.topMargin

            Layout.fillWidth: true
            Layout.preferredHeight: Math.min(layout.implicitHeight + layout.anchors.topMargin + layout.anchors.bottomMargin, flickable.maxheight)

            flickContentHeight: layout.implicitHeight + layout.anchors.topMargin + layout.anchors.bottomMargin

            navbarVisible: false

            ColumnLayout {
                id: layout
                anchors.fill: parent
                anchors.margins: MZTheme.theme.windowMargin * 1.5

                Repeater {
                    model: bottomSheet.model
                    delegate: Loader {
                        id: loader

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

                        sourceComponent: getSourceComponent()

                        Component {
                            id: titleBlock

                            MZBoldInterLabel {
                                Layout.fillWidth: true

                                text: loader.composerBlock.text
                                font.pixelSize: MZTheme.theme.fontSize
                                lineHeight: MZTheme.theme.labelLineHeight
                                verticalAlignment: Text.AlignVCenter
                                wrapMode: Text.Wrap
                            }
                        }

                        Component {
                            id: textBlock

                            MZInterLabel {

                                text: loader.composerBlock.text
                                font.pixelSize: MZTheme.theme.fontSizeSmall
                                color: MZTheme.theme.fontColor
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
