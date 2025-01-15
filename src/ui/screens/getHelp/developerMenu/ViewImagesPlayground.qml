
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

    MZMenu {
        id: menu
        // Do not translate this string!
        title: "Images playground"
        _menuOnBackClicked: () => getHelpStackView.pop()
    }

    ColumnLayout {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: animationContainer.bottom
        anchors.topMargin: MZTheme.theme.listSpacing * 3
        spacing: MZTheme.theme.listSpacing * 2
        width: parent.width - MZTheme.theme.windowMargin * 2

        MZComboBox {
            id: animationSelect

            currentIndex: -1
            // Do not translate this string!
            placeholderText: "Select an image"

            model: ListModel {
              Repeater {
        model: MZAssetLookup.imageLookup
        ListElement { 
          name: "blah"
          value: "blah2"
           }
    }
              // Component.onCompleted: {
              //     for (var i = 0; i < MZAssetLookup.imageLookup.length; i++) {
              //         ListElement {
              //           name: "HEYYY"
              //           value: ":/ui/resources/animations/vpnlogo-drop_animation.json"
              //         }
              //         // append(MZAssetLookup.imageLookup[i]);
              //     }
              // }
            }

            // delegate: Delegate {
            //     width: parent.width
            //     contentItem: Text {
            //         text: "Hey"//model.key
            //     }
            // }

            // delegate: Delegate {
            //     width: parent.width
            //     contentItem: ListElement {
            //         name: model.key
            //         value: model
            //     }
            // }

            // model: ListModel {
            //   Repeater {
            //   model: 
            //   delegate: ListElement {
            //         id: delegate
            //         required property string key
            //         name: modelData.key
            //         // value: modelData
            //     }
            // }
            // }

            // model: ListModel {
            //     id: imageItems

            //     ListElement {
            //         name: "VPN Logo (drop)"
            //         value: ":/ui/resources/animations/vpnlogo-drop_animation.json"
            //     }
            //     ListElement {
            //         name: "Lock"
            //         value: ":/ui/resources/animations/lock_animation.json"
            //     }
            //     ListElement {
            //         name: "Globe"
            //         value: ":/ui/resources/animations/globe_animation.json"
            //     }
            //     ListElement {
            //         name: "VPN Active"
            //         value: ":/ui/resources/animations/vpnactive_animation.json"
            //     }
            // }

            function setCurrentAnimationSource() {
                if (currentIndex >= 0) {
                    root.selectedAnimationSource = animationItems.get(currentIndex).value;
                }
            }

            onCurrentIndexChanged: () => {
                setCurrentAnimationSource()
            }

            Component.onCompleted: {
                setCurrentAnimationSource()
            }

            Layout.fillWidth: true
        }

    }

}
