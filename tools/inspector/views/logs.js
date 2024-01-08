/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { html, css, LitElement } from 'lit'
import { currentClient } from '../globalstate'
import { parseLogLine } from '../inspector/logsParser.js'

export class ViewLogs extends LitElement {
  static styles = css`
    :host{
        background:white;
        display:flex;
        flex-direction: column;
        width:100%;
        height:100%;
        overflow-y:auto;
    }
    main{
        height: 100%;
        max-height: 100%;
        overflow-y: auto;
        flex-grow: 8;
        flex: 5;
    }
    aside{
        display: flex;
        padding: 0px 20px;
    }
    ul{
        display: flex;
        flex-direction: row;
        flex-wrap: wrap;
        padding: 0;
        max-width: 30vw;
        justify-content: space-between;
    }
    ul >*{
        flex-grow:1;
    }
    input{
        padding: 10px 5px;
        border-radius: 10px;
        border: 1px solid black;
    }
    table{
        padding: 20px;  
        border-spacing: 10px 5px;
        width: 100%;
    }
    tr{
        margin: 20px 0px;
    }
    th{
      text-align: left;
    }
    
    `

  constructor(){
    super();
    currentClient.subscribe( c => c.qWebChannel.subscribe((q)=>this.clientChanged(q)))
  }
  clientChanged(client){
    if(!client){
        return;
    }
    this.logHandler = client.objects.MZLog;
    this.logHandler.logEntryAdded.connect(log => this.logLineAdded(log))
  }
  logLineAdded(log){
    const entry = parseLogLine(log);
    console.log(entry);
    this.logs.push(entry);
    this.requestUpdate('logs')
  }


  static properties = {
    filter: { type: String },
    logs: { attribute: false },
    components: { attribute: false }
  }

  connectedCallback () {
    super.connectedCallback()

    this.filter = ''
    this.logs = []
  }

  get renderlist () {
    let outList = this.logs
    if (this.filter) {
      const filter = this.filter.toLowerCase()
      outList = outList.filter((l) => l.text.toLowerCase().includes(filter)  || l.category.toLowerCase().includes(filter))
    }
    return outList
  }
  filterChanged(filter){
    this.filter = filter; 
    this.requestUpdate("logs")
  }
  clear(){
    this.logs = [];
    this.requestUpdate("logs")
  }

  render () {
    return html`
        <aside>
            <button @click=${()=>this.clear()}">clear </button>
            <input placeholder="filter" value="${this.filter}" 
              @change=${(e) => this.filterChanged(e.target.value)} 
              @keydown=${(e) => this.filterChanged(e.target.value)}/>
        </aside>
        <main>
            <table>
                <tr class="heading">
                    <th>Time</th>
                    <th>Component</th>
                    <th>Message</th>
                </tr>
                ${this.renderlist.map(row => html`
                  <tr>
                  <td>${row.date.getHours()}:${row.date.getMinutes()}:${row.date.getSeconds()}</td>
                  <td>${row.category}</td>
                  <td>${row.text}</td>
                  </tr>
                `)}
            </table>
        </main>
    `
  }
}
customElements.define('view-logs', ViewLogs)
