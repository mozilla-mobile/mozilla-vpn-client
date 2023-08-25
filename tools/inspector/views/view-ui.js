/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { html, css, LitElement } from 'lit'
import { UIObserver } from '../inspector/UiObserver'
import '../elements/pill-toggle'
import '../elements/live-view'

export class ViewUi extends LitElement {
  static styles = css`
    :host{
        background:white;
        display:flex;
        width:100%;
        height:100%;
        overflow-y:auto;
        flex-direction: column;
        padding: 20px;
    }
    main{
        height: 100%;
        overflow-y: auto;
        flex-grow:1;
        display:flex;
    }
    table{
        width:100%;
    }
    aside{
        height: 70vh;
        max-width: 50%;
        flex-grow: 1;
        display: flex;
        flex-direction: column;
        padding: 20px;
        border: 1px solid var(--lt-color-gray-900);
        background: white;
        max-height: 92vh;
        overflow-y: auto;
        overflow-x: auto;
        flex: 4;
        position: sticky;
        top: 2px;
        right: 30px;

    }
    aside > button {
      position: sticky;
      top: 0px;
    }
    aside > *{
        margin-top:5px;
    
    }
    ul{
        padding-left: 20px;
        list-style:none;
    }
    li{
        cursor: pointer;
    }
    li ul{
        padding-left: 29px;
    }
    li.collapsed ul{
        display:none;
    }
    li.active > span.name{
        background:blue;
        color:white;
    }
    li span.name{
        border-radius:10px;
        padding:3px;
    }

    tr{
        cursor: pointer;
    }
    pre{
        padding:0;
        margin:0;
    }
    tr.active{
      background: lightgray;
    }  
    #canvasHolder{
        flex:1;
        display:flex;
        justify-content:center;
        align-items:center;
        position: sticky;
        top: 20px;
    }
    live-view{
      position: sticky;
      top: 0px;
    }
    `

  constructor () {
    super()
    this.settings = {
      'Hide Invisible Elements': false,
      'Hide Null Size Elements': false,
      'Show Rulers': false
    }
  }

  connectedCallback () {
    super.connectedCallback()
    UIObserver.onAny(() => { this.onRequest() })
    requestIdleCallback(() => {
      UIObserver.refresh()
    })
  }

  static get properties () {
    return {
      data: { attribute: false },
      detail: { attribute: false },
      settings: { attribute: false }
    }
  }

  onRequest () {
    this.data = UIObserver.tree
    this.requestUpdate('data')
  }

  renderTree (rootObject) {
    if (rootObject.subItems.length == 0) {
      return ''
    }
    return html`
            <ul>
                ${rootObject.subItems.map(item => this.renderNode(item))}
            </ul>
        `
  }

  renderNode (node) {
    // Check if the Node Matches the Filter:
    if (this.settings['Hide Invisible Elements']) {
      if (!node.visible) {
        return html``
      }
    }
    if (this.settings['Hide Null Size Elements']) {
      if (node.width == 0 && node.height == 0) {
        return html``
      }
    }

    const cleanClassname = node.__class__.split('_QMLTYPE_')[0].split('_QML_')[0]

    const text = (node.objectName != '') ? node.objectName : cleanClassname
    const hasChilden = node.subItems.length > 0
    const isDetail = this.detail == node
    const isCollapsed = node.__collapsed__
    return html`
        <li class="view-element ${hasChilden ? 'hasChilden' : ''} ${isDetail ? 'active' : ''} ${isCollapsed ? 'collapsed' : ''}">
        ${hasChilden
? html`
            <span class="indicator" @click="${(e) => { this.toggleCollapse(e, node) }}">
            ${isCollapsed ? '➡️' : '⬇️'}
            </span>
        `
: ''}   
            
            <span class="name" @click="${(e) => { this.openDetail(e, node) }}" >${text}</span>  
            ${this.renderTree(node)}
        </li>
        `
  }

  renderDetail (node) {
    console.log(node)
    return html`
             <aside style="flex:1;">
                 <button @click="${() => this.closeDetail()}">close</button>
                 <hr>
                <table>
                    ${Object.entries(node).map(kv => {
                        const [key, value] = kv
                        return html`
                            <tr>
                                <td> ${key}</td>
                                <td><pre>${value}</pre></td>
                            </tr>
                        `
                    })}
                </table>
                <hr>
             </aside>
            `
  }

  openDetail (e, node) {
    e.stopPropagation()
    this.detail = node
    this.requestUpdate('detail')
  }

  closeDetail () {
    this.detail = undefined
  }

  quickFilterChange (id) {
    this.settings[id] = !this.settings[id]
    this.requestUpdate('settings')
  }
  saveImage(){
    this.renderRoot.querySelector("live-view").saveImage();
  }

  toggleCollapse (event, node) {
    node.__collapsed__ = !node.__collapsed__
    this.requestUpdate('data')
  }

  getElementCount(node) {
    if (!node) {
      return 0;
    }
    if (node.subItems.length == 0) {
      return 1;
    }
    return node.subItems.reduce((i, n) => i + this.getElementCount(n), 0)
  }

  render () {
    return html`
         <nav>
            <span>Total Elements: ${
        this.data ? this.getElementCount(this.data[0]) : 0}</span>
            <pill-toggle noActive="true"  @click=${(e) => {
      this.saveImage()
    }}>Download Image</pill-toggle>
            <span>Settings:</span>
            ${
        Object.keys(this.settings)
            .map(
                e => html` <pill-toggle id="${e}" @click=${
                    () => this.quickFilterChange(e)}>${e}</pill-toggle>`)}
        </nav>
        <main>
          <live-view .qmlHighlight=${this.detail} .showRulers=${
        this.settings['Show Rulers']}></live-view>
          <ul style="flex:1;">
            ${this.data ? this.renderNode(this.data[0]) : ''}
          </ul>
          ${this.detail ? this.renderDetail(this.detail) : ''}
        </main>
    `
  }
}
customElements.define('view-ui', ViewUi)
