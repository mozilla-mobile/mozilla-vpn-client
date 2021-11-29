/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
 
import { html, css, LitElement } from 'lit'

export class SimpleGreeting extends LitElement {
  static styles = css`p { color: blue }`

  static properties = {
    name: { type: String }
  }

  constructor () {
    super()
    this.name = 'Somebody'
  }

  render () {
    return html`<p>Hello, ${this.name}!</p>`
  }
}
customElements.define('simple-greeting', SimpleGreeting)
