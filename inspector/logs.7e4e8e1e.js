function t(t,e,i,s){Object.defineProperty(t,e,{get:i,set:s,enumerable:!0,configurable:!0})}var e=("undefined"!=typeof globalThis?globalThis:"undefined"!=typeof self?self:"undefined"!=typeof window?window:"undefined"!=typeof global?global:{}).parcelRequire6a04;e.register("HVM7K",(function(i,s){t(i.exports,"ViewLogs",(()=>r));var n=e("aD10K");e("kRfZQ"),e("iKuHS");var o=e("2KwL0");class r extends n.LitElement{static styles=n.css`
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
        flex-grow: 8;
        flex: 5;
    }
    aside{
        contain:strict;
        height: 100%;
        max-width: 20vw;
        min-width: 200px;
        flex-grow: 1;
        display: flex;
        flex-direction: column;
        padding: 20px;
        border-right: solid 1px var(--lt-color-gray-900);
        flex: 1;
    }
    aside > *{
        margin-top:10px;
    }
    aside p
    {
        margin-bottom: 0;
        font-weight: bold;
        margin-top: 40px;
    }

    ul{
        display: flex;
        flex-direction: row;
        flex-wrap: wrap;
        padding: 0;
        max-width: 30vw;
        justify-content: space-between;
    }
    ul >*{
        flex-grow:1;
    }
    input{
        padding: 10px 5px;
        border-radius: 10px;
        border: 1px solid var(--lt-color-gray-400);
    }
    table{
        padding: 20px;  
        border-spacing: 10px 5px;
    }
    .heading{
        position:sticky;
        top:0px;
        background:white;
        padding:20px;
    }
    tr{
        margin: 20px 0px;
    }
    
    `;static properties={filter:{type:String},logs:{attribute:!1},components:{attribute:!1}};connectedCallback(){super.connectedCallback(),this.filter="",this.logs=[],this.components=[],this.logs=o.LogsObserver.LOG_ELEMENTS,this.components=o.LogsObserver.LOG_COMPONENT,o.LogsObserver.on("newLogs",(t=>{this.requestUpdate("logs")})),o.LogsObserver.on("newComponent",(t=>{this.requestUpdate("components")}))}logRow(t){return n.html`
            <tr>
                <td>${t.date.getHours()}:${t.date.getMinutes()}:${t.date.getSeconds()}</td>
                <td>${t.component}</td>
                <td>${t.log}</td>
            </tr>
        `}get renderlist(){let t=this.logs;this.filter&&(t=t.filter((t=>t.raw.includes(this.filter))));const e=Array.from(this.renderRoot.querySelectorAll("pill-toggle.active")).map((t=>t.textContent));return e.length>0&&(t=t.filter((t=>e.includes(t.component)))),t}logComponentFilter(t){return n.html`
            <pill-toggle>${t}</pill-toggle>
        `}render(){return n.html`
        <aside>
            <record-button></record-button>
            <input placeholder="filter" value="${this.filter}" @change=${t=>this.filter=t.target.value}>
            <p>Components</p>
            <ul @click=${()=>{this.requestUpdate("logs")}}>
                ${this.components.map(this.logComponentFilter)}
            </ul>

        </aside>
        <main>
            <table>
                <tr class="heading">
                    <th>Time</th>
                    <th>Component</th>
                    <th>Message</th>
                </tr>
                ${this.renderlist.map((t=>this.logRow(t)))}
            </table>
        </main>
    `}}customElements.define("view-logs",r)})),e.register("iKuHS",(function(t,i){var s=e("aD10K"),n=e("2KwL0");class o extends s.LitElement{static styles=s.css`
    :host{
        margin:2px;
    }
    :host button{
        border-radius: 8px;
        padding: 6px;
        background:blue;
        color: white;
        width: 100%;
    }
    :host(.recording) button{
        background:red;
    }
    `;static properties={recording:{type:Boolean},content:{type:String}};constructor(){super(),this.recording=!1,this.setContent()}setContent(){this.recording?this.content="Stop recording":this.content="Start recording"}connectedCallback(){super.connectedCallback(),this.addEventListener("click",(()=>{if(this.classList.toggle("recording"),this.recording=!this.recording,this.setContent(),this.recording)n.LogsObserver.startRecording();else{const t=n.LogsObserver.stopRecording();this.saveLogs(t)}}))}render(){return s.html`<button>${this.content}</button>`}saveLogs(t){const e=document.createElement("a");document.body.appendChild(e),e.style="display: none";const i=new Blob([t.join("\n")],{type:"octet/stream"}),s=window.URL.createObjectURL(i);e.href=s,e.download="mozillavpn.txt",e.click(),window.URL.revokeObjectURL(s)}}customElements.define("record-button",o)})),e.register("2KwL0",(function(i,s){t(i.exports,"LogsObserver",(()=>l));var n=e("cud2N"),o=e("vbrOd");class r extends o.GenericDispatcher{constructor(){super(),this.LOG_ELEMENTS=[],this.LOG_MODULES=[],this.LOG_COMPONENT=[],this.recording=!1,this.rowLogEntries=[],n.Client.on("log",(t=>this.processLogLine(t.value)))}startRecording(){this.recording=!0}stopRecording(){this.recording=!1;const t=this.rowLogEntries;return this.rowLogEntries=[],t}parseDate(t){const e=t.split(" "),i=e[0].split(".");return new Date(`${i[2]}-${i[1]}-${i[0]}T${e[1]}`)}detectType(t,e){return/Set state:/.exec(e)&&t.includes("main")?TYPE_VPN_STATE:/Setting state:/.exec(e)&&t.includes("controller")?TYPE_CONTROLLER_STATE:null}processLogLine(t){if(this.recording&&this.rowLogEntries.push(t),0===t.length||"["!==t[0])return;const e=t.indexOf("]");if(-1===e)return;const i=t.slice(1,e),s=(t=t.slice(e+1).trim()).indexOf("(");if(-1===s)return;const n=(t=t.slice(s)).indexOf(")");if(-1===n)return;const o=t.slice(1,n),r=t.slice(n+1).trim(),l=o.indexOf(" - ");if(-1===l)return;const c=o.slice(0,l).split("|"),d=o.slice(l+3),a={date:this.parseDate(i),detectedType:this.detectType(c,r),modules:c,component:d,log:r};this.LOG_ELEMENTS.push(a),this.LOG_ELEMENTS.length>200&&this.LOG_ELEMENTS.shift();for(const t of c)this.LOG_MODULES.includes(t)||(this.LOG_MODULES.push(t),this.emit({type:"newModule",entry:t,list:this.LOG_MODULES}));this.LOG_COMPONENT.includes(d)||(this.LOG_COMPONENT.push(d),this.emit({type:"newComponent",entry:d,list:this.LOG_COMPONENT})),this.emit({type:"newLogs",entry:a,list:this.LOG_ELEMENTS})}}const l=new r}));
//# sourceMappingURL=logs.7e4e8e1e.js.map
