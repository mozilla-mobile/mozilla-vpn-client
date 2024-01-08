/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
 
 import { html, css, LitElement } from 'lit'
 import {Task} from '@lit/task';

 import "./qdom-property"


 export class QDomDetail extends LitElement {
   static styles = css`
   div{
    padding:10px;
   }

   :host{
    font-size:13px;
   }
   li{
        list-style:none;
   }
   `

    _propTask = new Task(this, {

    task: async ([target], {signal}) => {
      const staticProps = await target.staticProperties();
      const dynamicProps = await target.dynamicProperties();
      const forwardedProperties = unwrap(this.currentTarget);

      return [
        forwardedProperties,
        unwrap(staticProps),
        unwrap(dynamicProps)
      ]

    },

    args: () => [this.currentTarget]

  });
 
   static properties = {
     currentTarget: { type: Object },
   }
 
   render () {
        if(!this.currentTarget){
            return html``;
        }


        return this._propTask.render({
            pending: () => html`<p>Loading product...</p>`,
            complete: (props) => {
                const elements = props.flatMap((propList) => propList.map(renderProperties))
                return  html`
                <aside style="flex:1;">
                    <button @click="${() => this.close()}">close</button>
        
                    <div>
                    ${elements}
                    </div>
         
                </aside>`
            }
        })
    }
 }



 function unwrap(target){
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
    let type = "null"
    try{
        const value = property.value ? property.value : property.get();
        type = key.endsWith("Changed") ? "signal": typeof value;
    }catch(e){}

        switch(type){
            case 'signal':
            case 'function':
            case 'object':
            case 'undefined':
            case 'null':
                return
            default: 
                return html`
                    <qdom-property .target=${this.currentTarget} .currentProperty=${property} .key=${key} .disabled=${false}></qdom-property>
                `
        }
    

}


 customElements.define('qdom-detail', QDomDetail)
 