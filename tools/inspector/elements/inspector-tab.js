/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
 
import { html, css, LitElement } from 'lit'
import { CommandObserver } from '../inspector/CommandObserver.js'

export class InspectorTab extends LitElement {
  static properties = {
    tag: { type: String },
    import: { type: String },
    title: { type: String },
    command: { type: String },
    commandAvaliable: { type: Boolean }
  }

  static styles = css`
  :host{
    border: 2px solid transparent;
    margin:10px;
    padding-left: 10px;
    padding-right: 10px;
    border-radius:30px;
    transition: all 0.2s;    
  } 
  :host(.active) {
    border: 2px solid blue
  } 
  :host(.unavailable) {
    display:none !important
  } 

  :host(.active) {
    color: white;
    background: blue;
  }
  :host(.active) p{
    color: white;
  }  

  *{
    cursor: default;
  }
  
  p { 
    color: black;
    padding: 0px; 
    margin:5px;
  }`
  constructor () {
    super()
    this.title = this.innerText
    this.command == ''
    this.commandAvaliable = true

    CommandObserver.on('commandsChanged', (e) => {
      /** @type {Array} */
      const commandList = e.value
      if (this.command == '' || this.command == undefined) {
        this.commandAvaliable = true
        return
      }
      this.commandAvaliable = commandList.includes(this.command)
      this.classList.toggle('unavailable', !this.commandAvaliable)
    })
  }

  render () {
    return html`<p>${this.title}</p>`
  }
}
customElements.define('inspector-tab', InspectorTab)
