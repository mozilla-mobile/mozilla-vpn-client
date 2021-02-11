var e=("undefined"!=typeof globalThis?globalThis:"undefined"!=typeof self?self:"undefined"!=typeof window?window:"undefined"!=typeof global?global:{}).parcelRequire6a04;e.register("kRfZQ",(function(t,n){var o=e("aD10K");class i extends o.LitElement{static styles=o.css`
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
    
    `;static properties={text:{type:String},noActive:{type:Boolean}};constructor(){super(),this.name="Somebody",this.noActive=!1}connectedCallback(){super.connectedCallback(),this.name=this.innerText,this.addEventListener("click",(()=>{this.noActive||this.classList.toggle("active")}))}render(){return o.html`<button>${this.textContent}</button>`}}customElements.define("pill-toggle",i)}));
//# sourceMappingURL=logs.4a298cee.js.map
