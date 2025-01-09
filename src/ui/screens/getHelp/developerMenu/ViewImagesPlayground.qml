
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1
import "qrc:/nebula/utils/MZAssetLookup.js" as MZAssetLookup

Item {
    id: root
    property string selectedImageSourceLight
    property string selectedImageSourceDark

    MZMenu {
        id: menu
        // Do not translate this string!
        title: "Images playground"
        _menuOnBackClicked: () => getHelpStackView.pop()
    }

    Rectangle {
        id: imageContainer

        anchors.top: menu.bottom
        color: MZTheme.colors.bgColor
        height: 375
        width: parent.width

        MZInterLabel {
            // Do not translate this string!
            text: "No image selected"
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            color: MZTheme.colors.fontColor
            visible: imageSelect.currentIndex < 0
        }
        
        ColumnLayout {
          spacing: 10
          anchors.centerIn: parent
          RowLayout {
              spacing: 50
              // anchors.centerIn: parent
              Text {
                text: "Asset for light themes"
                color: MZTheme.colors.fontColor
                visible: imageSelect.currentIndex >= 0
              }

              Text {
                text: "Asset for dark themes"
                color: MZTheme.colors.fontColor
                visible: imageSelect.currentIndex >= 0
              }
          }
          RowLayout {
              spacing: 25
              Image {
                  source: selectedImageSourceLight
                  sourceSize.height: 150
                  sourceSize.width: 150
                  fillMode: Image.PreserveAspectFit
              }

              Image {
                  source: selectedImageSourceDark
                  sourceSize.height: 150
                  sourceSize.width: 150
                  fillMode: Image.PreserveAspectFit
              }
          }
          
            RowLayout {
              spacing: 25
              Rectangle {
                height: 150
                width: 150
                color: MZTheme.colors.fontColor
                visible: imageSelect.currentIndex >= 0
                Image {
                    source: selectedImageSourceLight
                    sourceSize.height: 150
                    sourceSize.width: 150
                    fillMode: Image.PreserveAspectFit
                }
              }
              
              Rectangle {
                height: 150
                width: 150
                color: MZTheme.colors.fontColor
                visible: imageSelect.currentIndex >= 0
              Image {
                  source: selectedImageSourceDark
                  sourceSize.height: 150
                  sourceSize.width: 150
                  fillMode: Image.PreserveAspectFit
              }
              }
            }
        }
        
    }

    ColumnLayout {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: imageContainer.bottom
        anchors.topMargin: MZTheme.theme.listSpacing * 3
        spacing: MZTheme.theme.listSpacing * 2
        width: parent.width - MZTheme.theme.windowMargin * 2

        MZComboBox {
            id: imageSelect

            currentIndex: -1
            // Do not translate this string!
            placeholderText: "Select an image"
            model: ListModel {
                id: imageItems
            }

            Component.onCompleted: {
                Object.keys(MZAssetLookup.imageLookup).forEach(function(imageRecord) {
                  imageItems.append({
                    name: imageRecord,
                    lightFile: MZAssetLookup.imageLookup[imageRecord].filenameLight,
                    darkFile: MZAssetLookup.imageLookup[imageRecord].filenameDark
                  });
                });
                setCurrentImageSource()
            }

            function setCurrentImageSource() {
                if (currentIndex >= 0) {
                    root.selectedImageSourceLight = imageItems.get(currentIndex).lightFile;
                    root.selectedImageSourceDark = imageItems.get(currentIndex).darkFile;
                }
            }

            onCurrentIndexChanged: () => {
                setCurrentImageSource()
            }

            Layout.fillWidth: true
        }

    }

}
