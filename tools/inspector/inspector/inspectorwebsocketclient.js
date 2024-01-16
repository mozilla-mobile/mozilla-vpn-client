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
   * @param {URL | string } url - The Websocket URL to connect to. 
   */
  constructor (url=DEFAULT_URL) {
    this.isConnected = signal(false)
    this.qWebChannel = signal()

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
    if(globalThis.WebSocket){
      // We're running in the browser or Deno
      this._setupWebSocket(WebSocket,url);
    }else {
      // Node i guess. 
      import('ws').then(ws => {
        this._setupWebSocket(ws.WebSocket,url)
      }
       
        );
    }
  }
  close(){
    this.isClosed = true;
    if(this.websocketConnection){
      this.websocketConnection.close()
    }
  }


  _setupWebSocket(websocketImpl,url){
    this.websocketConnection = new websocketImpl(url.toString());
    // 
    this.websocketConnection.onopen = () => {
      new QWebChannel(this.websocketConnection, (channel)=> {
        this.qWebChannel.value = channel;
        this.isConnected.value = true;
      });
    }
    this.websocketConnection.onerror = () => { 
      this.isConnected.value = false;
    }
    this.websocketConnection.onclose = () => { 
      this.isConnected.value = false;
      if(this.isClosed){
        return;
      }
      // If we're not closed by the calling code
      // retry to re-setup the connection. 
      setTimeout(()=>{
        this._setupWebSocket(websocketImpl,url);
      },200)
    }
  }
  /**
   * @type {WebSocket}
   */
  websocketConnection = null;

  /**
   * Signals whether the Inspector is connected.
   */
  isConnected = signal(false)

  isClosed= false;

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
