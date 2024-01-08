/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
 
 import { html, css, LitElement } from 'lit'
 import { Signal } from '@lit-labs/preact-signals';
 /**
  * QDomElement: 
  * 
  * Renders QDom Root element and expands rendering a list 
  * Of all QDOM elements
  * 
  * Properties
  * target -> The QQWrapper Element from the Inspector API
  * selectedElement -> channel<QQWrapper|null> that will contain the selected Element (will propagate downwards)
  * expanded -> whether the list is oened
  * 
  */
 export class QDomDetail extends LitElement {
   static styles = css`
   div{

   }

   :host{
    font-size:13px;
   }
   
   `
 
   static properties = {
     currentTarget: { type: Object },
   }
 
   render () {
        if(!this.currentTarget){
            return html``;
        }
        // [["key",{propertiy}]]
        const properties = getProperties(this.currentTarget);

        const elements = properties.map(renderProperties);
    return html`
            <aside style="flex:1;">
            <button @click="${() => this.close()}">close</button>
            <hr>
        <table>
            ${elements}
        </table>
            <hr>
        </aside>`
   }
 }



 function getProperties(target){
    return Object.entries(Object.getOwnPropertyDescriptors(target));
 }

 /**
  * 
  * @param {} property 
  * @returns 
  */
function renderProperties(kv){
    const [key, property]= kv;
    if(!key){
        return;
    }

    const value = property.value ? property.value : property.get();
    let type = key.endsWith("Changed") ? "signal": typeof value;
    const writable = property.set != undefined

    switch(type){
        case 'signal':
        case 'function':
            return html`<li>${key}: ${type}; </li>`
        default: 
            return html`
                <li>${key}: ${value} -> ${type} | ${writable}</li>
            `
    }
}


 customElements.define('qdom-detail', QDomDetail)
 