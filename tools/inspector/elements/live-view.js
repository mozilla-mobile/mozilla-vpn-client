/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { html, css, LitElement } from 'lit'
import { UIObserver } from '../inspector/UiObserver'

export class LiveView extends LitElement {
  static styles = css`p { color: blue }`

  static properties = {
    // QML Element to Highlight
    qmlHighlight: { attribute: false },
    // Decoded Image
    decodedImage: { attribute: false },
    // 2nd QML Elment to highlight
    qmlCompare: { attribute: false },
    // shows rulers
    showRulers: { attribute: false },
    // Time to Refresh ScreenShot in ms
    refreshTime: { attribute: false },
    // QML Root Element
    qmlRoot: { attribute: false }
  }

  constructor () {
    super()
    this.refreshTime = 1000
    this.showRulers = false
    this.qmlHighlight = null
  }

  saveImage(){
    const canvas = this.renderRoot.querySelector('canvas')
    var link = this.renderRoot.getElementById('link');
    link.setAttribute('download', 'Screenshot.png');
    link.setAttribute('href', canvas.toDataURL("image/png").replace("image/png", "image/octet-stream"));
    link.click();
  }

  connectedCallback () {
    super.connectedCallback()
    UIObserver.on('screenshot', (evnt) => {
      this.screen = evnt.detail.screen.value
      this.decodeImage()
    })
    UIObserver.on('tree', (evnt) => {
      const tree = evnt.detail.list
      this.qmlRoot = tree[0]
    })
    this.timer = window.setInterval(() => {
      UIObserver.getScreen()
    }, this.refreshTime)
  }

  disconnectedCallback () {
    super.disconnectedCallback()
    window.clearInterval(this.timer)
  }

  decodeImage () {
    const image = new Image()
    image.onload = () => {
      this.decodedImage = image
    }
    image.src = 'data:image/png;base64,' + this.screen
  }

  renderCanvas () {
    if (!this.renderRoot) {
      return
    }
    const canvas = this.renderRoot.querySelector('canvas')
    if(!canvas){
      return;
    }
    if(!this.decodedImage){
      return;
    }
    const ctx = canvas.getContext('2d')
    requestAnimationFrame(() => {
      ctx.drawImage(this.decodedImage, 0, 0, canvas.width, canvas.height)
      if (!this.qmlHighlight) {
        return
      }
      const { totalX, totalY, width, height } = this.qmlHighlight

      ctx.beginPath()
      ctx.fillStyle = 'rgba(255,165,0,0.3)'
      ctx.rect(totalX, totalY, width, height)
      ctx.fill()
      ctx.closePath()

      if (this.showRulers) {
        ctx.beginPath()
        ctx.strokeStyle = 'rgba(255,165,0,0.3)'

        ctx.moveTo(totalX, 0)
        ctx.lineTo(totalX, this.qmlRoot.height)

        ctx.moveTo(totalX + width, 0)
        ctx.lineTo(totalX + width, this.qmlRoot.height)

        ctx.moveTo(0, totalY)
        ctx.lineTo(0, this.qmlRoot.width)

        ctx.moveTo(0, totalY + height)
        ctx.lineTo(0, totalY + height + this.qmlRoot.width)

        ctx.stroke()
        ctx.closePath()
      }
    })
  }

  requestUpdate (name, oldValue) {
    if (name == 'qmlHighlight') {
      this.renderCanvas()
    }
    if (name == 'decodedImage') {
      this.renderCanvas()
    }
    if (name == 'showRulers') {
      this.renderCanvas()
    }
    return super.requestUpdate(name, oldValue)
  }

  render () {
    if (!this.qmlRoot) {
      return html``
    }
    return html`
       <div id="canvasHolder">
         <a id="link" style="display:none">downloadImage</a>
        <canvas style="background:pink;" width=${this.qmlRoot.width} height=${this.qmlRoot.height}></canvas>
       </div>
    `
  }
}
customElements.define('live-view', LiveView)
