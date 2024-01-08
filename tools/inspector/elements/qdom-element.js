/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { Signal } from '@lit-labs/preact-signals';
import { html, css, LitElement } from 'lit'

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
export class QDomElement extends LitElement {
    static styles = css`
   div{
        display:flex;
        width:100%
   }
    button{
        display: flex;
        background: none;
        border: none;
        align-items: center;
        transition: all ease 0.2s;
        padding: 0;
    }
    button+button{
        flex-grow: 1;
    }
    div:hover{
        background: #0000ff63;
    }
    div.selected{
        background: #0000ff63;
    }
    .expanded{
        transform: rotateZ(90deg) translateX(5px);
    }
    .gg-chevron-right{
        transition: all ease 0.2s;
        scale: 0.8;
        width: 15px;
        height: 15px;
    }
    ul{
        padding: 0;
        margin: 0;
        list-style: none;
        padding-left: 12px;
    }
   
   `

    static properties = {
        // QQML wrapper
        target: { type: Object },
        expanded: { type: Boolean },
        selectedElement: { type: Signal },
        isSelectedElement: { type: Boolean }

    }

    constructor() {
        super()
        this.expanded = true;
    }

    firstUpdated() {
        if (!this.selectedElement) {
            return;
        }
        this.selectedElement.subscribe(element => {
            if (element == this.target) {
                this.isSelectedElement = true;
                return;
            }
            this.isSelectedElement = false;
        })

    }
    onSelect() {
        if (!this.selectedElement) {
            return;
        }
        this.selectedElement.value = this.target;
        this.isSelectedElement = true;
    }

    // Renders the sublist if Expanded
    renderChildren() {
        if(!this.target){
            return;
        }
        if (!this.expanded) {
            return html``;
        }
        if(!this.target.children){
            return;
        }
        const children = this.target.children.map(
            c => html`<li>
                <qdom-element .target=${c} .selectedElement=${this.selectedElement}>
                </qdom-element>
                </li>
            `)
        return html`
            <ul>
                ${children}
            </ul>
        `;
    }
    // Renders the "expand button"
    // if target has children
    expandButton() {
        if (this.target?.children?.length == 0) {
            return html``;
        }
        return html`
        <button @click=${() => { this.expanded = !this.expanded }}>
            <i class="gg-chevron-right ${this.expanded ? "expanded" : ""}"></i>
        </button>
        `
    }

    elementSummary() {

        const qualifiedName = this.target?.className.split("_")[0]
        const url = this.target?.source
        const name = this.target?.objectName
        const text = this.target?.text
        return html`
        <button @click=${()=>this.onSelect()}>
        ${qualifiedName}
        ${url ? ` |src=${url}` : ""}
        ${name ? `|name=${name}` : ""}
        ${text ? `|text=${text}` : ""}
        </button>
     `
    }

    render() {
        return html`
        <div class=${this.isSelectedElement ? "selected" : ""}>
            <link href='https://unpkg.com/css.gg@2.0.0/icons/css/chevron-right.css' rel='stylesheet'>
            ${this.expandButton()}
            ${this.elementSummary()}
        </div>
        ${this.renderChildren()} 
        `
    }
}
customElements.define('qdom-element', QDomElement)
