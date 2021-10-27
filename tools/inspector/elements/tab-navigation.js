/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
 
import { html, css, LitElement } from 'lit'
import './inspector-tab.js'
import { loadView } from '../views/loader'

export class TabNavigation extends LitElement {
  static styles = css`
    :host {
      display: block;
      width: 100vw;
      background: white;
      margin:0;
    }

    nav{
      display: flex;
      width: 100%;
      justify-content: flex-start;
    }
  `

  static properties = {
    target: { type: String }
  }

  constructor () {
    super()
    this.updateComplete.then(() => {
      // Auto activate the one marked as active on first load
      const active = this.querySelector('inspector-tab.active')
      if (active) {
        this.activateTab(active)
      }
    })
  }

  async onClick (e) {
    /** @type {TabButton} */
    const tabButton = e.target.closest('inspector-tab')
    if (!tabButton) {
      return
    }
    this.activateTab(tabButton)
  }

  activateTab (tabButton) {
    const tag = tabButton.tag
    loadView(tag)
    const viewElement = document.createElement(tag)
    if (viewElement == null) {
      console.error(`Failed to create View Element requested by tab-button <${this.tabView}>`)
      return
    }
    /** @type {HTMLElement} */
    const box = document.querySelector(this.target)
    if (!box) {
      console.error(`Tabview can't find target element: ${this.target}`)
      return
    }
    box.innerHTML = ''
    box.appendChild(viewElement)

    // Inspector Tabs are in <slots> thus live in document not shadow root :)
    const old_active = document.querySelector('inspector-tab.active')
    if (old_active) {
      old_active.classList.remove('active')
    }
    tabButton.classList.add('active')
  }

  render () {
    return html`
      <nav @click=${(e) => this.onClick(e)}>
        <slot>

        </slot>
      </nav>
    `
  }
}
customElements.define('tab-navigation', TabNavigation)
