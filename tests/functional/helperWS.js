/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
import { InspectorWebsocketClient } from '@mozillavpn/inspector'

/** @type {InspectorWebsocketClient} */
let client;


/**
 * Set's up a QWebchannel via a Websocket connection. 
 * Connects to the hostname in options.
 * @param {URL} options 
 * @returns {Promise<InspectorWebsocketClient>}
 */
export async function connect(options) {
  const inspectorClient = new InspectorWebsocketClient(`ws://${options.hostname}:8765/`);

  return await new Promise(resolve => {
    inspectorClient.isConnected.subscribe(connected => {
      if(connected){
        console.log(`Connected to: ws://${options.hostname}:8765/`)
        client = inspectorClient;
        resolve(inspectorClient)
      }
    })
  });
}

export async function close() {
  if(!client){
    return;
  }
  return client.websocketConnection.close();
}
export async function send(msg) {
  if(!client){
    return;
  }
  return client.websocketConnection.send(msg);
}
