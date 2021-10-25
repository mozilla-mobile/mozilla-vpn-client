import { html, css, LitElement } from 'lit'
import { Client } from '../inspector/client'

export class ConnectionKillswitch extends LitElement {
  static styles = css`
    :host(){
        display:none;
    }
    :host(.active){
        display: flex;
        justify-content:center;
        align-items:center;
        width: 100vw;
        height:100vh;
        position: fixed;
        top: 0;
        left:0;
        background: #262626ad;
        backdrop-filter: blur(20px);
    }
    *{
        color:white;
        font-size:2em;
    }
  
  `

  static properties = {
    reason: { type: String }
  }

  constructor () {
    super()
    this.classList.add('active')
    this.reason = 'Connecting...'

    Client.on('connected', () => {
      this.classList.remove('active')
      this.reason = ''
    })
    Client.on('disconnected', () => {
      this.reason = 'Disconnected, please reload'
      this.classList.add('active')
    })
  }

  dismiss () {
    this.classList.remove('active')
  }

  render () {
    return html`<div @click=${e => this.dismiss()}>
        ${this.reason}
    </div>`
  }
}
customElements.define('connection-killswitch', ConnectionKillswitch)
