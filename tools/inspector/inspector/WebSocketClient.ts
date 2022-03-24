/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { GenericDispatcher } from './genericdispatcher'
import { IClient } from './IClient'


export const DEFAULT_URL = "ws://localhost:8765"
export class WebSocketClient extends GenericDispatcher implements IClient {
  queue: Array<any>
  url: string
  connectionPromise: Promise<string>
  websocketConnection: WebSocket

  constructor() {
    super()
    this.queue = []
  }

  type() {
    return "websocket";
  }


  connect(url = DEFAULT_URL) {
    if (url === this.url) {
      return this.connectionPromise
    }
    // This is a new connection! 
    // Close a connection if we have one. 
    if (this.websocketConnection) {
      this.websocketConnection.close();
    }

    this.emit({ type: 'connecting' });

    this.connectionPromise = new Promise((resolve, reject) => {
      console.log('Doing Connection!')
      this.websocketConnection = new WebSocket(url)
      this.websocketConnection.onopen = () => {
        this.emit({ type: 'connected' })
        resolve(this.websocketConnection.url)
        console.log('Connected to' + url)
        if (this.queue.length > 0) {
          this.queue.forEach(c => this.websocketConnection.send(c))
          this.queue = []
        }
      }
      this.websocketConnection.onerror = () => {
        this.emit({ type: 'connectionFailed' });
        reject();
        this.url = "";
      }
      this.websocketConnection.onclose = () => {
        this.emit({ type: 'disconnected' })
        this.url = "";
      }
      this.websocketConnection.onmessage = data => this.processMessage(data.data)
    })
    this.url = url;
    return this.connectionPromise
  }

  processMessage(data) {
    const json = JSON.parse(data)
    this.emit(json)
  }

  sendCommand(command) {
    if (this.websocketConnection.readyState != 1) {
      this.queue.push(command)
      return
    }
    this.websocketConnection.send(command)
  }

  currentURL(): string {
    return this.url;
  }
}