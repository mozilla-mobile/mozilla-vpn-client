/*
 * Copyright 2018 Kai Uwe Broulik <kde@broulik.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.9

/**
 * This item emulates an HTML <img> tag
 */
Image {
    // FIXME Why doesn't property alias src: source work?!
    //property alias src: source
    property string src
    source: src

    readonly property real naturalWidth: sourceSize.width
    readonly property real naturalHeight: sourceSize.height

    // Dummy HTML compat APIs
    property string crossOrigin
    property string alt
    readonly property bool complete: status === Image.Ready
    // FIXME why can't I alias this to source?!
    readonly property string currentSrc: src

    function addEventListener(event, cb) {
        switch(event) {
        case "load":
            __loadedCallbacks.push(cb);
            break;
        case "error":
            __errorCallbacks.push(cb);
            break;
        }
    }

    asynchronous: true
    visible: false

    property var __loadedCallbacks: []
    property var __errorCallbacks: []

    onStatusChanged: {
        if (status === Image.Error) {
            __errorCallbacks.forEach(Function.prototype.call, Function.prototype.call);
        } else if (status === Image.Ready) {
            __loadedCallbacks.forEach(Function.prototype.call, Function.prototype.call);
        }
    }
}
