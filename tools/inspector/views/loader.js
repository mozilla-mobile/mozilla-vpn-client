/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// Imports the Module that defines <tag></tag>
export async function loadView (tag) {
    switch(tag){
        case "view-shell":
            return import("./view-shell.js")
        case "view-logs":
            return import("./logs.js")
        case "view-network":
            return import("./view-network.js")
        case "view-ui":
            return import("./view-ui.js")
    } 
}
