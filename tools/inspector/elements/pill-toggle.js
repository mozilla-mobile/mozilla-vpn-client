/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { html, css, LitElement } from 'lit'

export class PillToggle extends LitElement {
  static styles = css`
    :host{
        margin:2px;
    }
    :host(.active) button{
        color: white; 
        background:blue;
    }
      button { 
          color: black;
      background: transparent;
      border-radius: 8px;
      border: solid 1px transparent;
      padding: 6px;
      transition: all 0.3s ease;
  
      }
      button:hover{
        color: white; 
        background:blue;
      }
    
    `

  static properties = {
    text: { type: String },
    noActive: {type: Boolean}
  }

  constructor () {
    super()
    this.name = 'Somebody'
    this.noActive=false;
  }

  connectedCallback () {
    super.connectedCallback()
    this.name = this.innerText
    this.addEventListener('click', () => {
      if(this.noActive){
        return;
      }
      this.classList.toggle('active')
    })
  }

  render () {
    return html`<button>${this.textContent}</button>`
  }
}
customElements.define('pill-toggle', PillToggle)
