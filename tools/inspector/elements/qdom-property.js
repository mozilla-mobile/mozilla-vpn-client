/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
 
 import { html, css, LitElement } from 'lit'
 
 /**
  * QDomProperty: 
  * 
  * Renders one Property 
  * Listens for changes. 
  * Allows modifications if it is settable. 
  * 
  */
 export class QDomProperty extends LitElement {
   static styles = css`
   div{

   }
   :host{
    font-size:13px;
    display:flex;
    flex-direction:row;
    width:100%;
    border-bottom: 1px solid black;
    align-items: center;
   }
   p,input{
    flex:1;
   }
   input{
    height:13px;
   }
   
   `
 
   static properties = {
      // QObject Targeted
     currentTarget: { type: Object },
     // property to be used. 
     currentProperty: {type: Object},
     // the key of the property
     key: {type:String},
     disabled:{type:Boolean}
   }
 
   render () {
    const property = this.currentProperty;
    let writable = property.set != undefined
    if(this.disabled){
        writable = false;
    }
    const value = property.value ? property.value : property.get();
    let type = this.key.endsWith("Changed") ? "signal": typeof value;


    if(!writable){
        return html`
        <p>${this.key}</p> 
        <p>${value}</p>
    `;
    }

    switch(type){
        case "boolean":
            return html`
                <p>
                    ${this.key}:
                </p>
                <input type="checkbox" checked=${value} @change=${this.handleInput}/>
            `;
        default:
            return html`
                <p>
                    ${this.key}:
                </p>
                <input type=${type} value=${value} @change=${this.handleInput}/>
            `;
        }
    }

    handleInput(event){
        const target = event.target; 
        if(this.currentProperty.set == undefined){
            return;
        }
        // Special case checkbox
        if(target.type ="checkbox"){
            this.currentProperty.set(target.checked)
            return;
        }
        if(this.currentProperty.set != undefined){
            this.currentProperty.set(target.value)
        }
    }
 }


customElements.define('qdom-property', QDomProperty)
 