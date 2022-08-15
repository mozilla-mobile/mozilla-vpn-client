/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */


import { Octokit } from "https://cdn.skypack.dev/@octokit/core";
const octokit = new Octokit({  });

const TASKCLUSTER_INSTANCE ="https://firefox-ci-tc.services.mozilla.com/"
const TASKCLUSTER_TASK_NAME = "build-wasm/opt";
/**
 *  <taskcluster-frame></taskcluster-frame>
 * 
 *  Custom Element that will load the wasm client from the given taskname 
 *  and a git commit sha. 
 *  Usage: 
 *  -> set element.git_sha="asdjko3131" 
 *  -> Profit.
 */
class TaskclusterFrame extends HTMLElement{
    
     /** @type {ShadowRoot} */
    #dom= null;
    /** @type {String} */
    #sha ="";
    /** @type {HTMLIFrameElement} */
    #iframe = null;
    /** @type {String} */
    taskID = "";

    artifact_url="";

    constructor(){
        super();
    }


    get git_sha() {
        return this.#sha;
    }
    set git_sha(value){
        this.#sha = value;
        this.update();
    }


    async connectedCallback(){
        this.#dom = this.attachShadow({mode: 'open'});
        this.render();
        this.update();
    }


    render(){
        if(this.#dom.innerHTML != ''){
            this.update();
        }
        this.#dom.innerHTML = `
            <style>
                iframe{
                    width: 100%;
                    height: 100%;
                }
            </style>
           <iframe src="about:blank"></iframe>
        `;
        this.#iframe= this.#dom.querySelector("iframe");   
        //this.#iframe.srcdoc="Select Branch :)";  
    }
    async update(){
        if(this.#sha == ""){
            return;
        }
        this.#iframe.src="about:blank";

        const wasm_run = await this.getData(this.#sha);
        if(wasm_run == undefined){ 
            this.#iframe.src="about:blank";
        }

        const task_url = wasm_run.details_url;
        const task_id = task_url.split("/").at(-1);

        this.taskID = task_id;
        this.artifact_url = `${TASKCLUSTER_INSTANCE}api/queue/v1/task/${task_id}/artifacts/public/build/index.html`;
        this.#iframe.src = this.artifact_url;

        this.dispatchEvent(new CustomEvent("taskChanged", {}));
    }



    async getData(sha){
        const maybe_stored = localStorage.getItem(sha);
        if(maybe_stored){
            return JSON.parse(maybe_stored);
        }

        const response = await octokit.request('GET /repos/{owner}/{repo}/commits/{ref}/check-runs', {
            owner: 'mozilla-mobile',
            repo: 'mozilla-vpn-client',
            ref: sha,
            per_page: 100
        });
        console.log(response);
        /** @type {Array<Object>} */
        const checks = response.data.check_runs;
        const wasm_run = checks.find(e => e.name == TASKCLUSTER_TASK_NAME);
        if(!wasm_run){
            return undefined;
        }
        // The SHA -> Taskcluster Mapping is pretty permanent
        localStorage.setItem(sha, JSON.stringify(wasm_run));
        return wasm_run;
    }
}

customElements.define("taskcluster-frame", TaskclusterFrame);
console.log("BranchSelector loaded");