/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0

import compat 0.1
import "qrc:/nebula/utils/MZAssetLookup.js" as MZAssetLookup

Item {
    id: logoRoot

    property string avatarUrl

    Image {
        id: avatar

        anchors.centerIn: parent
        asynchronous: true
        height: logoRoot.height
        fillMode: Image.PreserveAspectFit
        smooth: true
        source: isDefaultAvatar() ? "" : avatarUrl
        visible: false
    }

    Image {
        id: avatarPlaceholder

        property int avatarSourceSize: 320

        anchors.fill: parent
        fillMode: Image.PreserveAspectFit
        smooth: true
        source: MZAssetLookup.getImageSource("AvatarProfile")
        sourceSize.height: avatarSourceSize
        sourceSize.width: avatarSourceSize
        visible: avatar.status !== Image.Ready
    }

    Rectangle {
        id: avatarMask

        anchors.fill: avatar
        radius: height / 2
        visible: false
    }

    MZOpacityMask {
        anchors.fill: avatar
        source: avatar
        maskSource: avatarMask
    }

    /**
     * We check if the id of the avatar is a just a single character
     * and assume that the user did not set a custom avatar
     */
    function isDefaultAvatar() {
        const avatarUrlSplitted = avatarUrl.split("/");
        const avatarID = avatarUrlSplitted[avatarUrlSplitted.length - 1];

        return avatarID && avatarID.length <= 1;
    }
}
