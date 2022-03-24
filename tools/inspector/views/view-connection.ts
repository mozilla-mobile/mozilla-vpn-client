import { html, css, LitElement } from 'lit'
import {customElement, property, query} from 'lit/decorators.js';
import { Client } from '../inspector/Client';

const STATE_CONNECTING = "connecting";
const STATE_CONNECTED = "connected";
const STATE_DISCONNECTED = "disconnected";
const HISTORY_KEY = "connection_history";

const CONNECTING_EMOJIS = ['🤖','✨','🌍','⏳','⌚','⏰',]

@customElement('view-connection')
export class ViewConnection extends LitElement {
    history: Array<string> = [];
    mTarget:string;
    @property({type:String})
    state:String = STATE_DISCONNECTED;

    constructor(){
        super();
        let storage_history= localStorage.getItem(HISTORY_KEY);
        if(storage_history){
          this.history = JSON.parse(storage_history);
        } else {
            this.history = [
                "ws://localhost:8765",
            ];
        }

        Client.on("connected", () => {
          this.state =STATE_CONNECTED;
          this.requestUpdate();
        });
        Client.on("disconnected", () => {
          this.state =STATE_DISCONNECTED;
          this.requestUpdate();
        });
    
    }
    submitConnect(){
        let url = (<HTMLInputElement>this.shadowRoot.getElementById("connectInput")).value;
        this.connectTo(url);
    }
    connectTo(input:string){
        let url: URL;
        try {
            url = new URL(input)
        } catch (error) {
            alert("The URL is not valid!");
        }
        if(!url){
            return;
        }
        // Save the new entry to the history
        if(!this.history.includes(url.href) && url.protocol == "ws:"){
            this.history.push(url.href);
            localStorage.setItem(HISTORY_KEY, JSON.stringify(this.history));
        }
      Client.connect(url.toString());
      this.state=STATE_CONNECTING;
      this.requestUpdate();
      this.mTarget= input;
    }

    connectedCallback(){
        super.connectedCallback();
    }

    render() {
        return html` 
            <div id="root">
                ${this.getTemplate(this.state)}
            </div>
        `;
      }
      getTemplate(state){
        switch(state){
          case STATE_DISCONNECTED:
            return this.disconnectedTemplate();
          case STATE_CONNECTING:
            return  this.connectingTemplate();
          case STATE_CONNECTED:
            return  this.connectedTemplate();
        }
      }

    maybeParent(){
      if(window.opener == null && window.parent == null){
        return html``;
      }
      let url ="http://wasm-parent/"
      return html`
        <li @click="${()=>this.connectTo("http://wasm-parent/")}" class="${Client.currentURL() == url ? 'active': ''}">
                <span>Wasm Client</span>
                <button>-></button>
        </li> 
      `;
    }

    historyTemplate(){
        return html`
          <div class="history">
           <h2>Past Connections</h2>
            <ul>
              ${this.maybeParent()}
              ${this.history.map(url => html`
              <li @click="${()=>this.connectTo(url)}" class="${Client.currentURL() == url ? 'active': ''}">
                <span>${url}</span>
                <button>-></button>
              </li> 
              `)}
            </ul>
          </div>
        `
    }
    connectedTemplate(){
      return html`
        <div id="connectingBox">
          <h1>Connected to </h1>
          <p>${this.mTarget}</p>
        </div>
      `
    }
  
    
  connectingTemplate(){
    return html`
      <div id="connectingBox">
        <h1>Connecting</h1>
        <p id="waiting">${CONNECTING_EMOJIS.at(Math.round(Math.random() * CONNECTING_EMOJIS.length ))}</p>
      </div>
    `
  }

  
  disconnectedTemplate(){
    return html`
    <div class="holder">
        <h1>Connect / Start</h1>
        <div class="connect-form">
          <input id="connectInput" type="text" placeholder="Inspector Address">
          <button @click="${() => this.submitConnect()}">-></button>
        </div>
        ${this.history.length >0 ? this.historyTemplate():""}
    </div>
    `
  }

  static styles = css`
    #root{
        position:relative;
        width:100%;
        height:100%;
        display:flex;
        justify-content:center;
        align-items:center;
        background: blue;
    }
    #drag-element{
        position:absolute;
        right:0;
        width: 2px;
        height: 100%;
        background: black;
        cursor: col-resize;
    }

    #connectingBox{
        display:flex;
        flex-direction:column;
        justify-content:center;
    }
    #connectingBox *{
      text-align: center;
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
  width: 90%;
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


  #waiting{
    animation: grow 1.5s linear infinite;
  }
  @keyframes grow {
    0% {
      transform: scale(1);
    }
    50% {
      transform: scale(5);
    }
    100% {
      transform: scale(1);
    }
  }
  `
}