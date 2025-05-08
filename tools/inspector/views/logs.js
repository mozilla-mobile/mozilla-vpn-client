/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { html, css, LitElement } from 'lit'
import '../elements/pill-toggle.js'
import '../elements/record-button.js';

import { LogsObserver } from '../inspector/LogsObserver.js'

export class ViewLogs extends LitElement {
  static styles = css`
    :host{
        background:white;
        display:flex;
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
        contain:strict;
        height: 100%;
        max-width: 20vw;
        min-width: 200px;
        flex-grow: 1;
        display: flex;
        flex-direction: column;
        padding: 20px;
        border-right: solid 1px var(--lt-color-gray-900);
        flex: 1;
    }
    aside > *{
        margin-top:10px;
    }
    aside p
    {
        margin-bottom: 0;
        font-weight: bold;
        margin-top: 40px;
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
        border: 1px solid var(--lt-color-gray-400);
    }
    table{
        padding: 20px;  
        border-spacing: 10px 5px;
    }
    .heading{
        position:sticky;
        top:0px;
        background:white;
        padding:20px;
    }
    tr{
        margin: 20px 0px;
    }
    
    `

  static properties = {
    filter: { type: String },
    logs: { attribute: false },
    components: { attribute: false }
  }

  connectedCallback () {
    super.connectedCallback()

    this.filter = ''
    this.logs = []
    this.components = []

    this.logs = LogsObserver.LOG_ELEMENTS
    this.components = LogsObserver.LOG_COMPONENT

    LogsObserver.on('newLogs', (e) => {
      this.requestUpdate('logs')
    })
    LogsObserver.on('newComponent', (e) => {
      this.requestUpdate('components')
    })
  }

  logRow (row) {
    return html`
            <tr>
                <td>${row.date.getHours()}:${row.date.getMinutes()}:${row.date.getSeconds()}</td>
                <td>${row.component}</td>
                <td>${row.log}</td>
            </tr>
        `
  }

  get renderlist () {
    let outList = this.logs
    if (this.filter) {
      outList = outList.filter((l) => l.raw.includes(this.filter))
    }
    const activePills = Array.from(this.renderRoot.querySelectorAll('pill-toggle.active'))
    const filterComponents = activePills.map(e => e.textContent)
    if (filterComponents.length > 0) {
      outList = outList.filter(row => filterComponents.includes(row.component))
    }
    return outList
  }

  logComponentFilter (component) {
    return html`
            <pill-toggle>${component}</pill-toggle>
        `
  }

  render () {
    return html`
        <aside>
            <record-button></record-button>
            <input placeholder="filter" value="${this.filter}" @change=${(e) => this.filter = e.target.value}>
            <p>Components</p>
            <ul @click=${() => { this.requestUpdate('logs') }}>
                ${this.components.map(this.logComponentFilter)}
            </ul>

        </aside>
        <main>
            <table>
                <tr class="heading">
                    <th>Time</th>
                    <th>Component</th>
                    <th>Message</th>
                </tr>
                ${this.renderlist.map(row => this.logRow(row))}
            </table>
        </main>
    `
  }
}
customElements.define('view-logs', ViewLogs)
