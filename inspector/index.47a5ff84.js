var t="undefined"!=typeof globalThis?globalThis:"undefined"!=typeof self?self:"undefined"!=typeof window?window:"undefined"!=typeof global?global:{},e={},n={},i=t.parcelRequire6a04;null==i&&((i=function(t){if(t in e)return e[t].exports;if(t in n){var i=n[t];delete n[t];var o={id:t,exports:{}};return e[t]=o,i.call(o.exports,o,o.exports),o.exports}var s=new Error("Cannot find module '"+t+"'");throw s.code="MODULE_NOT_FOUND",s}).register=function(t,e){n[t]=e},t.parcelRequire6a04=i);var o=i("aD10K"),s=i("cud2N");class c extends o.LitElement{static properties={history:{attribute:!1},state:{type:String}};constructor(){super(),this.state=this.classList.add("active"),this.state="disconnected",this.history=[s.DEFAULT_URL];let t=localStorage.getItem("connection_history");t&&(this.history=JSON.parse(t)),s.Client.on("connected",(()=>{this.classList.remove("active"),this.state="connected"})),s.Client.on("connecting",(()=>{this.classList.add("active"),this.state="connecting"})),s.Client.on("disconnected",(()=>{this.classList.add("active"),this.state="disconnected"})),s.Client.on("connectionFailed",(()=>{this.classList.add("active"),this.state="disconnected"}))}render(){switch(this.state){case"disconnected":return this.disconnectedTemplate();case"connected":case"connecting":return this.connectingTemplate()}}connectTo(t){s.Client.connect(t),this.history.includes(t)||(this.history.push(t),localStorage.setItem("connection_history",JSON.stringify(this.history)))}submitConnect(){let t=this.renderRoot.querySelector("#connectInput").value;t&&this.connectTo(t)}connectingTemplate(){return o.html`
      <h1>Connecting...</h1>
      <p>Todo: Add a spinner?</p>
    `}historyTemplate(){return o.html`
      <div class="history">
       <h2>Past Connections</h2>
        <ul>
          ${this.history.map((t=>o.html`
          <li @click="${()=>this.connectTo(t)}">
            <span>${t}</span>
            <button>-></button>
          </li> 
          `))}
        </ul>
      </div>
    `}disconnectedTemplate(){return o.html`
    <div class="holder">
        <h1>Disconnected Please Connect</h1>
        <div class="connect-form">
          <input id="connectInput" type="text" placeholder="Inspector Address">
          <button @click="${()=>this.submitConnect()}">-></button>
        </div>
        ${this.history.length>0?this.historyTemplate():""}
    </div>
    `}static styles=o.css`
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
        z-index: 100;
    }
    *{
        color:white;
        font-size:1em;
    }
    .holder{
      display:flex;
      justify-content:center;
      flex-direction: column;
      align-items: initial;
      min-width: 40vw;
    }

    .connect-form{
      flex-grow:1;
      display:flex;
    }

    .connect-form button{
      border-radius:30px;
      border:none;
      color:white;
      background-color:blue;
      padding:10px;
      margin-left: 10px;
    }
    .connect-form input{
      border-radius:30px;
      border:none;
      color:black;
      padding:10px;
      flex-grow:1
    }

    .history{
      background: white;
      color: black;
      padding: 10px;
      margin: 10px;
      border-radius: 10px;
      margin: 20px 0px;
    }
    .history *{
      color:black;
    }
    .history button{
      border-radius:30px;
      border:none;
      color:black;
      padding:10px;
      background: transparent;
    }
    .history ul{
      list-style:none;
      padding:0;
    }
    .history li{
      border: 1px solid blue;
      padding: 5px;
      border-radius: 3px;
    }

`}customElements.define("connection-killswitch",c);
//# sourceMappingURL=index.47a5ff84.js.map
