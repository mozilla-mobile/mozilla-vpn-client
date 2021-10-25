import { html, css, LitElement } from 'lit'

export class SimpleGreeting extends LitElement {
  static styles = css`
  
:host{
    width:100%;
    height:100%;
    display:flex;
    justify-content:center;
    align-items:center;
}
    img{
        flex-grow:1;
    }
  
  `

  render () {
    return html`
       <img src="https://media.giphy.com/media/T8Dhl1KPyzRqU/source.gif">
    `
  }
}
customElements.define('view-todo', SimpleGreeting)
