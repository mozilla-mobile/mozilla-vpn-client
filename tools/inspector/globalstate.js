/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
import { signal } from '@lit-labs/preact-signals';

import { InspectorWebsocketClient } from './inspector/inspectorwebsocketclient'

/**
* The Current connected Client
* @{InspectorWebsocketClient}
*/
const currentClient = signal(new InspectorWebsocketClient())



export {
    currentClient
}


globalThis.state = {
    currentClient
}
