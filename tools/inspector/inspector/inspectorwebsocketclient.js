/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
import { QWebChannel } from 'qwebchannel'
import { signal } from '@lit-labs/preact-signals';

/**
 * InspectorWebsocketClient
 * 
 * Connects to the VPN Client using a Websocket, 
 * qWebChannel can be subscribed to once ready. 
 */
class InspectorWebsocketClient {
  /**
   * Creates a new InspectorClient
   * @param {URL} url - The Websocket URL to connect to. 
   */
  constructor (url=DEFAULT_URL) {
    this.isConnected = signal(false)
    this.qWebChannel = signal(null)

    if(!url){
      return;
    }
    if( typeof url != URL){
      url = new URL(url);
    }
    if(url.protocol =! "ws"){
      console.error("Invalid url");
    }
    this.url = url;
    this.websocketConnection = new WebSocket(url.toString());
    // 
    this.websocketConnection.onopen = () => {
      this.isConnected.value = true;
    }
    this.websocketConnection.onerror = () => { 
      this.isConnected.value = false;
    }
    this.websocketConnection.onclose = () => { 
      this.isConnected.value = false;
    }
    this.isConnected.subscribe(connected =>{
       if(connected){
        new QWebChannel(this.websocketConnection, (channel)=> {
          this.qWebChannel.value = channel;
        });
       }
    })

  }
  /**
   * Signals whether the Inspector is connected.
   */
  isConnected = signal(false)

   /**
   * The current qWebChannel
   * is either null or the current qWebChannel. 
   */
   qWebChannel = signal(null)
}

/**
 * Default Websocket URL
 */
const DEFAULT_URL ="ws://localhost:8765"

export {
  InspectorWebsocketClient,
  DEFAULT_URL
};
