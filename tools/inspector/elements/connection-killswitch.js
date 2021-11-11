/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { html, css, LitElement } from 'lit'
import { Client,DEFAULT_URL } from '../inspector/client'

const STATE_CONNECTING = "connecting";
const STATE_CONNECTED = "connected";
const STATE_DISCONNECTED = "disconnected";

const HISTORY_KEY = "connection_history";

export class ConnectionKillswitch extends LitElement {
  static properties = {
    history: { attribute: false },
    state: { type: String }
  }

  constructor () {
    super()
    this.state = 
    this.classList.add('active')
    this.state= STATE_DISCONNECTED;
    this.history=[DEFAULT_URL];

    let storage_history= localStorage.getItem(HISTORY_KEY);
    if(storage_history){
      this.history = JSON.parse(storage_history);
    }

    Client.on('connected', () => {
      this.classList.remove('active')
      this.state = STATE_CONNECTED;
    })
    Client.on('connecting', () => {
      this.classList.add('active')
      this.state = STATE_CONNECTING;
     })
    Client.on('disconnected', () => {
     this.classList.add('active')
     this.state = STATE_DISCONNECTED;
    })
    Client.on('connectionFailed', () => {
      this.classList.add('active')
      this.state = STATE_DISCONNECTED;
     })
  }

  render () {
    switch(this.state){
      case STATE_DISCONNECTED:
        return this.disconnectedTemplate();
      case STATE_CONNECTED:
      case STATE_CONNECTING:
        return this.connectingTemplate();
        
    }
  }
  connectTo(url){
    Client.connect(url);
    if(this.history.includes(url)){
      return;
    }
    // Not in the history yet, so add it!
    this.history.push(url);
    localStorage.setItem(HISTORY_KEY, JSON.stringify(this.history))
  }
  submitConnect(){
    let value = this.renderRoot.querySelector("#connectInput").value;
    if(!value){
      return;
    }
    this.connectTo(value);
  }

  connectingTemplate(){
    return html`
      <h1>Connecting...</h1>
      <p>Todo: Add a spinner?</p>
    `
  }

  historyTemplate(){
    return html`
      <div class="history">
       <h2>Past Connections</h2>
        <ul>
          ${this.history.map(url => html`
          <li @click="${()=>this.connectTo(url)}">
            <span>${url}</span>
            <button>-></button>
          </li> 
          `)}
        </ul>
      </div>
    `
  }
  disconnectedTemplate(){
    return html`
    <div class="holder">
        <h1>Disconnected Please Connect</h1>
        <div class="connect-form">
          <input id="connectInput" type="text" placeholder="Inspector Address">
          <button @click="${() => this.submitConnect()}">-></button>
        </div>
        ${this.history.length >0 ? this.historyTemplate():""}
    </div>
    `
  }

  static styles = css`
    :host(){
        display:none;
    }
    :host(.active){
        display: flex;
        justify-content:center;
        align-items:center;
        width: 100vw;
        height:100vh;
        position: fixed;
        top: 0;
        left:0;
        background: #262626ad;
        backdrop-filter: blur(20px);
        z-index: 100;
    }
    *{
        color:white;
        font-size:1em;
    }
    .holder{
      display:flex;
      justify-content:center;
      flex-direction: column;
      align-items: initial;
      min-width: 40vw;
    }

    .connect-form{
      flex-grow:1;
      display:flex;
    }

    .connect-form button{
      border-radius:30px;
      border:none;
      color:white;
      background-color:blue;
      padding:10px;
      margin-left: 10px;
    }
    .connect-form input{
      border-radius:30px;
      border:none;
      color:black;
      padding:10px;
      flex-grow:1
    }

    .history{
      background: white;
      color: black;
      padding: 10px;
      margin: 10px;
      border-radius: 10px;
      margin: 20px 0px;
    }
    .history *{
      color:black;
    }
    .history button{
      border-radius:30px;
      border:none;
      color:black;
      padding:10px;
      background: transparent;
    }
    .history ul{
      list-style:none;
      padding:0;
    }
    .history li{
      border: 1px solid blue;
      padding: 5px;
      border-radius: 3px;
    }

`


}
customElements.define('connection-killswitch', ConnectionKillswitch)
