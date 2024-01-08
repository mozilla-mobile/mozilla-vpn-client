/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { Client } from './inspector/inspectorwebsocketclient'

import "./elements/tab-navigation";
import "./elements/connection-killswitch";


import {html, css, LitElement} from 'lit';

export class InspectorApp extends LitElement {
  static styles = css`
      :host{
        display:flex;
        flex-direction: column;
        background: yellow;
        overflow:hidden;
      }

      connection-killswitch{
        display: none;
      }
      connection-killswitch.active{
        display: flex;
      }
      main{
        flex-grow:1;
        background:black;
      }      
  `;

  static properties = {
    name: {type: String},
  };

  constructor() {
    super();
    this.name = 'Somebody';
  }

  firstUpdated() {
    this.tabNavigation = this.renderRoot.querySelector('tab-navigation');
    this.main = this.renderRoot.querySelector('main');

    this.tabNavigation.target = this.main;
  }

  render() {
    return html`
      <connection-killswitch></connection-killswitch>
      <tab-navigation target="main">
          <inspector-tab  class="active" tag="view-shell">🐚 Shell</inspector-tab>
          <inspector-tab  tag="view-logs">📑 Logs</inspector-tab>
          <inspector-tab  tag="view-ui" command="view_tree">✨ QML Inspector</inspector-tab>
      </tab-navigation>
      <main>
      
      </main>
      
    
    
    `;
  }
}
customElements.define('mz-inspector', InspectorApp);
