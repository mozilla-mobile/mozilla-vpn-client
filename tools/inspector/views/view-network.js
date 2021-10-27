/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
 
import { html, css, classMap, LitElement } from 'lit'

import { NetworkObserver } from '../inspector/NetworkObserver'

export class ViewNetwork extends LitElement {
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
        flex-grow:1;
    }
    table{
        width:100%;
        padding: 40px;
        border-spacing: 10px 40px;
    }
    aside{
        height: 100%;
        max-width:50%;
        flex-grow:1;
        display:flex;
        flex-direction: column;
        padding:20px;
        transition: all 0.2s ease;
        contain: strict;
        border: solid 5px blue;
        background: white;
        color: black;

        position: absolute;
        width: 50vw;
   
        z-index: 20;
        right: -100vw;
        height: 90vh;
        top: 0;
        bottom: 0;
        margin: auto;
        padding: 20px;
        border-radius: 20px;
    }
    aside.active{
        right: 2vw;
    }
    aside section{
        display: flex;
        flex-direction: column;
    }
    aside > *{
        margin-top:5px;
    
    }

    aside ul{
        display: flex;
        flex-direction: row;
        flex-wrap: wrap;
        padding: 0;
        max-width: 30vw;
        justify-content: space-between;
    }
    aside ul >*{
        flex-grow:1;
    }

    .requestList{
        list-style: none;
        display: flex;
        flex-direction: column;
        justify-content: center;
        align-content: center;
        padding: 20px;
        position: relative;
    }
    .requestList li{
        margin: 2px;
        transition: all 0.2s;
        cursor: pointer;
        border-radius: 30px;
        display: flex;
        justify-content: space-between;
        overflow: hidden;
    }
    .requestList li.active{
        background:blue;
    }
    .requestList li.error{
        background:deeppink;
    }
    .requestList li.active *{
        color:white;
    }
    .requestList li span{
        padding: 10px;
    }
    .requestList li .status{
        flex:1;
        overflow: hidden;
    }
    .requestList li .host{
        flex:2;
        flex-shrink: 3;
        overflow: hidden;
        flex-grow: 5;
    }
    .requestList li .path{
        flex:4;
    }
    .requestList li .initator{
        flex:3;
        overflow: hidden;
    }

    
    `

  static properties = {
    filter: { type: String }
  }

  connectedCallback () {
    super.connectedCallback()
    NetworkObserver.onAny(() => { this.onRequest() })
  }

  onRequest () {
    this.requestUpdate()
  }

  openDetail (event, packet) {
    event.preventDefault()
    if (packet == this.detail) {
      return
    }
    this.detail = packet
    this.requestUpdate()
  }

  closeDetail () {
    this.detail = undefined
    this.requestUpdate()
  }

  requestRow (packet) {
    const me = this
    return html`
            <li @click="${(e) => { me.openDetail(e, packet) }}" class="${(this.detail == packet) ? 'active' : ''} ${packet.response.status != '200' ? 'error' : ''}">
                <span class="status"> ${packet.response.status}</span>
                <span class="host"> ${packet.request.url.host}</span>
                <span class="path"> ${packet.request.url.pathname}</span>
                <span class="initator"> ${packet.request.initiator}</span>
            </li>
        `
  }

  renderDetail (p) {
    let packet = p
    if (packet == undefined) {
      packet = {
        response: {
          status: '',
          headers: {
            'Conent-Type': 'text/plain'
          },
          body: ''
        },
        request: {
          url: '',
          initator: '',
          headers: []
        }
      }
    }
    const classes = { active: packet == undefined }
    const contentType = packet.response.headers['Content-Type']

    return html`
         <aside class="${p == undefined ? '' : 'active'}">
             <button @click="${() => this.closeDetail()}">close</button>
             <hr>
            <section>
                <p>Status: ${packet.response.status}</p>
                <p>URL: ${packet.request.url}</p>
                <p>From: ${packet.request.initiator}</p>
            </section>
            <hr>
            <section>
                <p>Request headers:</p>
                <pre>${packet.request.headers.map(r => r + '\n')}</pre>
            </section>
            <section>
                <p>Response headers:</p>
                <pre>${Object.keys(packet.response.headers).map(key => {
                    return key + ':' + packet.response.headers[key] + '\n'
                })}</pre>
            </section>
            <section>
                <p>Body: </p>
                <a href="data:${contentType};base64,${this.utf8_to_b64(packet.response.body)}" target="_blank">Inspect ↗️</a>
                ${this.renderBody(contentType, packet.response.body)}
            </section>
         </aside>
        `
  }

  utf8_to_b64 (str) {
    return window.btoa(unescape(encodeURIComponent(str)))
  }

  renderBody (type, body) {
    return html`
        <iframe src="data:${type};base64,${this.utf8_to_b64(body)}"></iframe>
        `
  }

  render () {
    let extra = ''
    // We only keep 50 requests, so if the detail-request was already removed
    // from the list, render it at the top
    if (this.detail && !NetworkObserver.requests.includes(this.detail)) {
      extra = this.requestRow(this.detail)
    }
    return html`
        <main>
            <ul class="requestList">
                ${extra}
                ${NetworkObserver.requests.map((r) => this.requestRow(r))}
            </ul>
        </main>
        ${this.renderDetail(this.detail)}
    `
  }
}
customElements.define('view-network', ViewNetwork)
