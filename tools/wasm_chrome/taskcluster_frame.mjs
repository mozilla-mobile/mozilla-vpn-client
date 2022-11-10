/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { Octokit } from "https://cdn.skypack.dev/@octokit/core";
const octokit = new Octokit({});
const static_branch_info = fetch("./branch_runs.json").then((r) => r.json());

const TASKCLUSTER_INSTANCE = "https://firefox-ci-tc.services.mozilla.com/";
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
class TaskclusterFrame extends HTMLElement {
  /** @type {ShadowRoot} */
  #dom = null;
  /** @type {Object} */
  #targetInternal = {
    sha: "",
    name: "",
  };
  /** @type {HTMLIFrameElement} */
  #iframe = null;
  /** @type {String} */
  taskID = "";

  #static_info = {};

  artifact_url = "";

  constructor() {
    super();
  }

  get target() {
    return this.#targetInternal;
  }
  set target(value) {
    this.#targetInternal = value;
    this.update();
  }

  async connectedCallback() {
    this.#dom = this.attachShadow({ mode: "open" });
    this.render();
    this.update();

    this.#static_info = [];
    try {
      this.#static_info = await static_branch_info;
    } catch (error) {
      console.error(error);
    }
  }

  render() {
    if (this.#dom.innerHTML != "") {
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
    this.#iframe = this.#dom.querySelector("iframe");
    //this.#iframe.srcdoc="Select Branch :)";
  }
  async update() {
    if (this.#targetInternal == undefined) {
      return;
    }
    if (this.loadFromIndex(this.#targetInternal.name)) {
      // Loaded this via target.
      return;
    }
    this.taskID = await this.getTaskID(this.#targetInternal.sha);
    if (this.taskID == "") {
      alert("Unable to find a Task for this branch");
      return;
    }

    this.#iframe.src =
      `${TASKCLUSTER_INSTANCE}api/queue/v1/task/${this.taskID}/artifacts/public/build/index.html`;
    this.dispatchEvent(new CustomEvent("taskChanged", {}));
  }

  // Tries to load branch $name from the taskcluster index.
  // Returns false if the build is not on it.
  loadFromIndex(name) {
    if (name === "main") {
      this.#iframe.src =
        `https://firefox-ci-tc.services.mozilla.com/api/index/v1/task/mozillavpn.v2.mozilla-vpn-client.branch.main.latest.build.wasm/artifacts/public%2Fbuild%2Findex.html`;
      return true;
    }
    // Todo: use tc namespaces api
    return false;
  }

  async getTaskID(sha) {
    // Before doing any api call, see if we have a ref for that sha in the static files.
    const static_data = this.#static_info[sha];
    if (
      static_data && static_data.task_status == "ok" &&
      static_data.task_id != ""
    ) {
      return static_data.task_id;
    }

    const wasm_run = await this.getGithubRun(sha);
    if (wasm_run == undefined) {
      return "";
    }
    const task_url = wasm_run.details_url;
    const task_id = task_url.split("/").at(-1);
    return task_id;
  }

  async getGithubRun(sha) {
    const maybe_stored = localStorage.getItem(sha);
    if (maybe_stored) {
      return JSON.parse(maybe_stored);
    }

    const response = await octokit.request(
      "GET /repos/{owner}/{repo}/commits/{ref}/check-runs",
      {
        owner: "mozilla-mobile",
        repo: "mozilla-vpn-client",
        ref: sha,
        per_page: 100,
      },
    );
    console.log(response);
    /** @type {Array<Object>} */
    const checks = response.data.check_runs;
    const wasm_run = checks.find((e) => e.name == TASKCLUSTER_TASK_NAME);
    if (!wasm_run) {
      return undefined;
    }
    // The SHA -> Taskcluster Mapping is pretty permanent
    localStorage.setItem(sha, JSON.stringify(wasm_run));
    return wasm_run;
  }
}

customElements.define("taskcluster-frame", TaskclusterFrame);
console.log("BranchSelector loaded");
