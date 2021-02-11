function e(e,t,i,s){Object.defineProperty(e,t,{get:i,set:s,enumerable:!0,configurable:!0})}var t=("undefined"!=typeof globalThis?globalThis:"undefined"!=typeof self?self:"undefined"!=typeof window?window:"undefined"!=typeof global?global:{}).parcelRequire6a04;t.register("2WOyT",(function(i,s){e(i.exports,"ViewUi",(()=>n));var l=t("aD10K"),r=t("dONv4");t("kRfZQ"),t("iuE2v");class n extends l.LitElement{static styles=l.css`
    :host{
        background:white;
        display:flex;
        width:100%;
        height:100%;
        overflow-y:auto;
        flex-direction: column;
        padding: 20px;
    }
    main{
        height: 100%;
        overflow-y: auto;
        flex-grow:1;
        display:flex;
    }
    table{
        width:100%;
    }
    aside{
        height: 100%;
        max-width: 50%;
        flex-grow: 1;
        display: flex;
        flex-direction: column;
        padding: 20px;
        border-left: solid 1px var(--lt-color-gray-900);
        background: white;
        position: sticky;
        top: 10px;
        max-height: 92vh;
        overflow-y: auto;
    }
    aside > *{
        margin-top:5px;
    
    }
    ul{
        padding-left: 20px;
        list-style:none;
    }
    li{
        cursor: pointer;
    }
    li ul{
        padding-left: 29px;
    }
    li.collapsed ul{
        display:none;
    }
    li.active > span.name{
        background:blue;
        color:white;
    }
    li span.name{
        border-radius:10px;
        padding:3px;
    }

    tr{
        cursor: pointer;
    }
    pre{
        padding:0;
        margin:0;
    }
    tr.active{
      background: lightgray;
    }  
    #canvasHolder{
        flex:1;
        display:flex;
        justify-content:center;
        align-items:center;
        position: sticky;
        top: 20px;
    }
    `;constructor(){super(),this.settings={"Hide Invisible Elements":!1,"Hide Null Size Elements":!1,"Show Rulers":!1}}connectedCallback(){super.connectedCallback(),r.UIObserver.onAny((()=>{this.onRequest()})),requestIdleCallback((()=>{r.UIObserver.refresh()}))}static get properties(){return{data:{attribute:!1},detail:{attribute:!1},settings:{attribute:!1}}}onRequest(){this.data=r.UIObserver.tree,this.requestUpdate("data")}renderTree(e){return 0==e.subItems.length?"":l.html`
            <ul>
                ${e.subItems.map((e=>this.renderNode(e)))}
            </ul>
        `}renderNode(e){if(this.settings["Hide Invisible Elements"]&&!e.visible)return l.html``;if(this.settings["Hide Null Size Elements"]&&0==e.width&&0==e.height)return l.html``;const t=e.__class__.split("_QMLTYPE_")[0].split("_QML_")[0],i=""!=e.objectName?e.objectName:t,s=e.subItems.length>0,r=this.detail==e,n=e.__collapsed__;return l.html`
        <li class="view-element ${s?"hasChilden":""} ${r?"active":""} ${n?"collapsed":""}">
        ${s?l.html`
            <span class="indicator" @click="${t=>{this.toggleCollapse(t,e)}}">
            ${n?"➡️":"⬇️"}
            </span>
        `:""}   
            
            <span class="name" @click="${t=>{this.openDetail(t,e)}}" >${i}</span>  
            ${this.renderTree(e)}
        </li>
        `}renderDetail(e){return console.log(e),l.html`
             <aside style="flex:1;">
                 <button @click="${()=>this.closeDetail()}">close</button>
                 <hr>
                <table>
                    ${Object.entries(e).map((e=>{const[t,i]=e;return l.html`
                            <tr>
                                <td> ${t}</td>
                                <td><pre>${i}</pre></td>
                            </tr>
                        `}))}
                </table>
                <hr>
             </aside>
            `}openDetail(e,t){e.stopPropagation(),this.detail=t,this.requestUpdate("detail")}closeDetail(){this.detail=void 0}quickFilterChange(e){this.settings[e]=!this.settings[e],this.requestUpdate("settings")}saveImage(){this.renderRoot.querySelector("live-view").saveImage()}toggleCollapse(e,t){t.__collapsed__=!t.__collapsed__,this.requestUpdate("data")}render(){return l.html`
         <nav>
            <pill-toggle noActive="true"  @click=${e=>{this.saveImage()}}>Download Image</pill-toggle>
            <span>Settings:</span>
            ${Object.keys(this.settings).map((e=>l.html` <pill-toggle id="${e}" @click=${()=>this.quickFilterChange(e)}>${e}</pill-toggle>`))}
        </nav>
        <main>
          <live-view .qmlHighlight=${this.detail} .showRulers=${this.settings["Show Rulers"]}></live-view>
          <ul style="flex:1;">
            ${this.data?this.renderNode(this.data[0]):""}
          </ul>
          ${this.detail?this.renderDetail(this.detail):""}
        </main>
    `}}customElements.define("view-ui",n)})),t.register("dONv4",(function(i,s){e(i.exports,"UIObserver",(()=>a));var l=t("cud2N"),r=t("vbrOd");class n extends r.GenericDispatcher{constructor(){super(),l.Client.on("qml_tree",(e=>this.onIncomingViewTree(e))),l.Client.on("view_tree",(e=>this.onIncomingViewTree(e))),l.Client.on("screen_capture",(e=>this.onIncomingScreen(e))),this.counter=0,this.requests=[]}fixTree(e){return e.map((e=>this.fixElement(e))),e}fixElement(e,t){e.parent=t,e.__collapsed__=!1,t?(e.totalX=e.x+t.totalX,e.totalY=e.y+t.totalY):(e.totalX=e.x,e.totalY=e.y),e.subItems&&e.subItems.forEach((t=>this.fixElement(t,e)))}onIncomingViewTree(e){this.tree=this.fixTree(e.tree),this.emit({type:"tree",list:this.tree})}onIncomingScreen(e){console.log("Incoming screenshot"),this.screen=e.value,this.emit({type:"screenshot",screen:e})}refresh(){l.Client.sendCommand("view_tree"),l.Client.sendCommand("screen_capture")}getScreen(){l.Client.sendCommand("screen_capture")}}const a=new n})),t.register("iuE2v",(function(e,i){var s=t("aD10K"),l=t("dONv4");class r extends s.LitElement{static styles=s.css`p { color: blue }`;static properties={qmlHighlight:{attribute:!1},decodedImage:{attribute:!1},qmlCompare:{attribute:!1},showRulers:{attribute:!1},refreshTime:{attribute:!1},qmlRoot:{attribute:!1}};constructor(){super(),this.refreshTime=1e3,this.showRulers=!1,this.qmlHighlight=null}saveImage(){const e=this.renderRoot.querySelector("canvas");var t=this.renderRoot.getElementById("link");t.setAttribute("download","Screenshot.png"),t.setAttribute("href",e.toDataURL("image/png").replace("image/png","image/octet-stream")),t.click()}connectedCallback(){super.connectedCallback(),l.UIObserver.on("screenshot",(e=>{this.screen=e.screen.value,this.decodeImage()})),l.UIObserver.on("tree",(e=>{const t=e.list;this.qmlRoot=t[0]})),this.timer=window.setInterval((()=>{l.UIObserver.getScreen()}),this.refreshTime)}disconnectedCallback(){super.disconnectedCallback(),window.clearInterval(this.timer)}decodeImage(){const e=new Image;e.onload=()=>{this.decodedImage=e},e.src="data:image/png;base64,"+this.screen}renderCanvas(){if(!this.renderRoot)return;const e=this.renderRoot.querySelector("canvas"),t=e.getContext("2d");requestAnimationFrame((()=>{if(t.drawImage(this.decodedImage,0,0,e.width,e.height),!this.qmlHighlight)return;const{totalX:i,totalY:s,width:l,height:r}=this.qmlHighlight;t.beginPath(),t.fillStyle="rgba(255,165,0,0.3)",t.rect(i,s,l,r),t.fill(),t.closePath(),this.showRulers&&(t.beginPath(),t.strokeStyle="rgba(255,165,0,0.3)",t.moveTo(i,0),t.lineTo(i,this.qmlRoot.height),t.moveTo(i+l,0),t.lineTo(i+l,this.qmlRoot.height),t.moveTo(0,s),t.lineTo(0,this.qmlRoot.width),t.moveTo(0,s+r),t.lineTo(0,s+r+this.qmlRoot.width),t.stroke(),t.closePath())}))}requestUpdate(e,t){return"qmlHighlight"==e&&this.renderCanvas(),"decodedImage"==e&&this.renderCanvas(),"showRulers"==e&&this.renderCanvas(),super.requestUpdate(e,t)}render(){return this.qmlRoot?s.html`
       <div id="canvasHolder">
         <a id="link" style="display:none">downloadImage</a>
        <canvas style="background:pink;" width=${this.qmlRoot.width} height=${this.qmlRoot.height}></canvas>
       </div>
    `:s.html``}}customElements.define("live-view",r)}));
//# sourceMappingURL=view-ui.4adc6734.js.map
