/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { html, css, LitElement } from 'lit'
import { UIObserver } from '../inspector/uiObserver'
import { currentClient } from '../globalstate'

import { InspectorWebsocketClient } from '../inspector/inspectorwebsocketclient'


import '../elements/pill-toggle'
import '../elements/live-view'
import '../elements/qdom-element'
import '../elements/qdom-detail'

import { signal } from '@lit-labs/preact-signals'


export class ViewUi extends LitElement {
  static styles = css`
    :host{
        background:white;
        display:flex;
        width:100%;
        height:100%;
        overflow-y:auto;
        flex-direction: column;
        padding: 0px 20px;
    }
    main{
        height: 100%;
        overflow-y: auto;
        flex-grow:1;
        display:flex;
        contain:strict;
    }
    qdom-element{
      overflow-y: auto;
}
qdom-detail{
  overflow-y: auto;
}
    `

  constructor () {
    super()
    currentClient.subscribe( c => c.qWebChannel.subscribe((q)=>this.clientChanged(q)))
    this.settings = {
      'Hide Invisible Elements': false,
      'Hide Null Size Elements': false,
      'Show Rulers': false
    }

    this.selectedElement = signal(null);
    this.selectedElement.subscribe(v =>{
      console.log(v);
      const detail = this.renderRoot?.querySelector("qdom-detail");
      if(detail){
        detail.currentTarget = v;
      }
     
    });
  }
  /**
   * Called whenever a new QWebchannel is available. 
   * @param {qWebChannel} client 
   */
  clientChanged(qWebChannel){
    this.webchannel = qWebChannel;
    this.qDOM = qWebChannel.objects.inspector_graph

    this.qDOM.queryAllElements().then((dom)=>{
      // I do not expect more then 1 Root Element, 
      // but leaving a TODO here, as we could. 
      this.rootQMLObject = dom[0];
      console.log(this.rootQMLObject)
    })
  }



  connectedCallback () {
    super.connectedCallback()
  }

  static get properties () {
    return {
      data: { attribute: false },
      detail: { attribute: false },
      settings: { attribute: false },
      rootQMLObject: { attribute: false },
    }
  }

  getElementCount(node) {
    if (!node) {
      return 0;
    }
    if (node.children.length == 0) {
      return 1;
    }
    return node.children.reduce((i, n) => i + this.getElementCount(n), 0)
  }

  render () {
    return html`
         <nav>
            <span>Total Elements: ${
        this.rootQMLObject ? this.getElementCount(this.rootQMLObject) : 0}</span>
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
        <qdom-element .target=${this.rootQMLObject} .selectedElement=${this.selectedElement}  ></qdom-element>
        <qdom-detail .selectedElement=${this.selectedElement}></qdom-detail>
        </main>
    `
  }
}
customElements.define('view-ui', ViewUi)
