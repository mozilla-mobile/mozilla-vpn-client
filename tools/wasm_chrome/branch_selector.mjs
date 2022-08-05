import { Octokit } from "https://cdn.skypack.dev/@octokit/core";
const octokit = new Octokit({  });


/**
 * 
 * <branch-selector></branch-selector>
 * 
 * BranchSelector is a custom element, extendins a <select> element, that
 * allows the user to select a branch from a list of branches, from the Mozilla-VPN repo
 * 
 * currently caches the branches for 24h ... needs an api key to get around that
 * 
 * fires a "change" event when the user selects a branch
 * the current branch is stored in the "value" property of the element
 */
class BranchSelector extends HTMLElement{
    /** @type {ShadowRoot} */
    #dom = null;

    #data = [];

    value = "";

    #firedOnload= false;

    constructor(){
        super();
    }


    async connectedCallback(){
        this.#dom = this.attachShadow({mode: 'open'});
        this.render();
        this.#data = await this.getData();
        this.update();
    }


    async getData(){
        const maybeStored = localStorage.getItem("branches");

        if(maybeStored){
            const stored = JSON.parse(maybeStored);
            const stored_age = new Date(stored.date) - new Date();
            // The branches should be cached for 1 hour
            if(stored_age < 1000 * 60 * 60 * 1){
                console.log("Using Cached Data");
                return stored.data;
            }
        }
        const response = await octokit.request('GET /repos/{owner}/{repo}/branches', {
            owner: 'mozilla-mobile',
            repo: 'mozilla-vpn-client',
            per_page: 100

        });

        console.log(`RateLimit -> ${response.headers["x-ratelimit-remaining"]}`)

        localStorage.setItem("branches", JSON.stringify({
            date: new Date(),
            data: response.data
        }));
        return response.data;
    }

    render(){
        if(this.#dom.innerHTML != ''){
            this.update();
        }
        this.#dom.innerHTML = `
            <select id="selector">
                    <option value="">Select a VPN</option>
            </select>
        `;
        const selector = this.#dom.querySelector("#selector");
        selector.addEventListener("change", e => {
            // Forward the event to the parent element;
            this.value = selector.value;
            this.name = selector.options[selector.selectedIndex].text;
            this.dispatchEvent(new CustomEvent("change", e));
        });
    }
    update(){
        const selector = this.#dom.querySelector("#selector");
        selector.innerHTML = `
            <option value="">Select a VPN-Branch</option>
            ${this.#data.map((e)=>{
                return `<option value="${e.commit.sha}">${e.name}</option>`	
            })};
        `;

        if(!this.#firedOnload){
            requestIdleCallback(()=>{
                const url = new URL(window.location);
                let name = url.searchParams.get('branch');
                if(!name){
                    this.#firedOnload=true;
                    return;
                }
                const option = Array.from(selector.querySelectorAll(`option`)).find(e => e.text == name);
                if(option){
                    option.selected = true;
                    this.value = option.value;
                    this.name = option.text;
                    this.dispatchEvent(new CustomEvent("change", {}));
                }
            });
        }

        
    }
}

customElements.define("branch-selector", BranchSelector);
console.log("BranchSelector loaded");