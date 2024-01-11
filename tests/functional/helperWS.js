/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
import { w3cwebsocket as websocket } from 'websocket';

let client;

export async function connect(options, onopen, onclose, onmessage) {
  return await new Promise(resolve => {
    client = new websocket(`ws://${options.hostname}:8765/`, '');

    client.onopen = async () => {
      await onopen();
      resolve(true);
    };

    client.onclose = () => onclose();
    client.onerror = () => resolve(false);

    client.onmessage = data => onmessage(JSON.parse(data.data));
  });
}
export async function close() {
  return client.close();
}
export async function send(msg) {
  return client.send(msg);
}
