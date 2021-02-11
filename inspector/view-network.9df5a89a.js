function e(e,t,s,i){Object.defineProperty(e,t,{get:s,set:i,enumerable:!0,configurable:!0})}var t=("undefined"!=typeof globalThis?globalThis:"undefined"!=typeof self?self:"undefined"!=typeof window?window:"undefined"!=typeof global?global:{}).parcelRequire6a04;t.register("loEoV",(function(s,i){e(s.exports,"ViewNetwork",(()=>o));var r=t("aD10K"),n=t("bXM9a");class o extends r.LitElement{static styles=r.css`
    :host{
        background:white;
        display:flex;
        width:100%;
        height:100%;
        overflow-y:auto;
    }
    main{
        height: 100%;
        max-height: 100%;
        overflow-y: auto;
        flex-grow:1;
    }
    table{
        width:100%;
        padding: 40px;
        border-spacing: 10px 40px;
    }
    aside{
        height: 100%;
        max-width:50%;
        flex-grow:1;
        display:flex;
        flex-direction: column;
        padding:20px;
        transition: all 0.2s ease;
        contain: strict;
        border: solid 5px blue;
        background: white;
        color: black;

        position: absolute;
        width: 50vw;
   
        z-index: 20;
        right: -100vw;
        height: 90vh;
        top: 0;
        bottom: 0;
        margin: auto;
        padding: 20px;
        border-radius: 20px;
    }
    aside.active{
        right: 2vw;
    }
    aside section{
        display: flex;
        flex-direction: column;
    }
    aside > *{
        margin-top:5px;
    
    }

    aside ul{
        display: flex;
        flex-direction: row;
        flex-wrap: wrap;
        padding: 0;
        max-width: 30vw;
        justify-content: space-between;
    }
    aside ul >*{
        flex-grow:1;
    }

    .requestList{
        list-style: none;
        display: flex;
        flex-direction: column;
        justify-content: center;
        align-content: center;
        padding: 20px;
        position: relative;
    }
    .requestList li{
        margin: 2px;
        transition: all 0.2s;
        cursor: pointer;
        border-radius: 30px;
        display: flex;
        justify-content: space-between;
        overflow: hidden;
    }
    .requestList li.active{
        background:blue;
    }
    .requestList li.error{
        background:deeppink;
    }
    .requestList li.active *{
        color:white;
    }
    .requestList li span{
        padding: 10px;
    }
    .requestList li .status{
        flex:1;
        overflow: hidden;
    }
    .requestList li .host{
        flex:2;
        flex-shrink: 3;
        overflow: hidden;
        flex-grow: 5;
    }
    .requestList li .path{
        flex:4;
    }
    .requestList li .initator{
        flex:3;
        overflow: hidden;
    }

    
    `;static properties={filter:{type:String}};connectedCallback(){super.connectedCallback(),n.NetworkObserver.onAny((()=>{this.onRequest()})),n.NetworkObserver.start()}onRequest(){this.requestUpdate()}openDetail(e,t){e.preventDefault(),t!=this.detail&&(this.detail=t,this.requestUpdate())}closeDetail(){this.detail=void 0,this.requestUpdate()}requestRow(e){const t=this;return r.html`
            <li @click="${s=>{t.openDetail(s,e)}}" class="${this.detail==e?"active":""} ${"200"!=e.response.status?"error":""}">
                <span class="status"> ${e.response.status}</span>
                <span class="host"> ${e.request.url.host}</span>
                <span class="path"> ${e.request.url.pathname}</span>
                <span class="initator"> ${e.request.initiator}</span>
            </li>
        `}renderDetail(e){let t=e;null==t&&(t={response:{status:"",headers:{"Conent-Type":"text/plain"},body:""},request:{url:"",initator:"",headers:[]}});const s=t.response.headers["Content-Type"];return r.html`
         <aside class="${null==e?"":"active"}">
             <button @click="${()=>this.closeDetail()}">close</button>
             <hr>
            <section>
                <p>Status: ${t.response.status}</p>
                <p>URL: ${t.request.url}</p>
                <p>From: ${t.request.initiator}</p>
            </section>
            <hr>
            <section>
                <p>Request headers:</p>
                <pre>${t.request.headers.map((e=>e+"\n"))}</pre>
            </section>
            <section>
                <p>Response headers:</p>
                <pre>${Object.keys(t.response.headers).map((e=>e+":"+t.response.headers[e]+"\n"))}</pre>
            </section>
            <section>
                <p>Body: </p>
                <a href="data:${s};base64,${this.utf8_to_b64(t.response.body)}" target="_blank">Inspect ↗️</a>
                ${this.renderBody(s,t.response.body)}
            </section>
         </aside>
        `}utf8_to_b64(e){return window.btoa(unescape(encodeURIComponent(e)))}renderBody(e,t){return r.html`
        <iframe src="data:${e};base64,${this.utf8_to_b64(t)}"></iframe>
        `}render(){let e="";return this.detail&&!n.NetworkObserver.requests.includes(this.detail)&&(e=this.requestRow(this.detail)),r.html`
        <main>
            <ul class="requestList">
                ${e}
                ${n.NetworkObserver.requests.map((e=>this.requestRow(e)))}
            </ul>
        </main>
        ${this.renderDetail(this.detail)}
    `}}customElements.define("view-network",o)})),t.register("bXM9a",(function(s,i){e(s.exports,"NetworkObserver",(()=>a));var r=t("cud2N"),n=t("vbrOd");class o extends n.GenericDispatcher{constructor(){super(),r.Client.on("network",(e=>this.onIncomingRequest(e))),this.counter=0,this.requests=[]}onIncomingRequest(e){const{request:t,response:s}=e;t.url=new URL(t.url),this.counter++,this.requests.push({id:this.counter,request:t,response:s}),this.requests.length>50&&this.requests.shift(),this.emit({type:"update",list:this.requests})}get(e){return this.requests[e]}start(){console.log("Start network Recording"),r.Client.sendCommand("fetch_network")}}const a=new o}));
//# sourceMappingURL=view-network.9df5a89a.js.map
