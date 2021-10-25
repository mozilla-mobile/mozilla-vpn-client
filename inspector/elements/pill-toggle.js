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
    text: { type: String }
  }

  constructor () {
    super()
    this.name = 'Somebody'
  }

  connectedCallback () {
    super.connectedCallback()
    this.name = this.innerText
    this.addEventListener('click', () => {
      this.classList.toggle('active')
    })
  }

  render () {
    return html`<button>${this.textContent}</button>`
  }
}
customElements.define('pill-toggle', PillToggle)
