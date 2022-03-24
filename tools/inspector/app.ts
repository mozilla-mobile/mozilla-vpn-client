import { html, css, LitElement } from 'lit'
import {customElement} from 'lit/decorators.js';

import {Client} from './inspector/Client';

import './elements/tab-navigation.js';

@customElement('app-inspector')
export class InspectorShell extends LitElement {

    constructor(){
        super();
        let search = window.location.search
        const args = new URLSearchParams(search);

        const connect = args.get("connect");
        console.log("Starting connection with " + connect);
        Client.connect(connect);
    }

    static styles = css`
        :host{
            flex: 1;
            flex-grow:1;
        }
        #root{
            position:relative;
            width:100%;
            height:100%;
            display:flex;
            justify-content: flex-start;
            align-items: center;
            flex-direction: column;
        }
        main{
            flex-grow:1;
            width: 100%;
            position: relative;
        }
    `

    connectedCallback(){
        super.connectedCallback();

       

    }

    firstUpdated(changedProperties){
      super.firstUpdated(changedProperties)
      if(Client.type() == "wasm"){
        this.shadowRoot.querySelectorAll("inspector-tab")[1].classList.add("active");
      }else{
        this.shadowRoot.querySelectorAll("inspector-tab")[0].classList.add("active");
      }
    }

    render() {
        return html` 
        <div id="root">
            <tab-navigation target="main">
                <inspector-tab tag="view-connection">🔗Connection</inspector-tab>
                <inspector-tab tag="view-shell">🐚 Console</inspector-tab>
                <inspector-tab  tag="view-logs">📑 Logs</inspector-tab>
                <inspector-tab  tag="view-network" command="fetch_network">📡 Network Inspector</inspector-tab>
                <inspector-tab  tag="view-ui" command="view_tree">✨ QML Inspector</inspector-tab>
            </tab-navigation>
            <main></main>
        </div>
        `;
    
      }
}