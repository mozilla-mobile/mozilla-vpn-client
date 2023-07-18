/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import "./fluent_components.mjs";
import { getGithubPaginatedData } from "./helpers.mjs";
import { Octokit } from "https://esm.sh/@octokit/core";

const octokit = new Octokit({});

const static_branch_info = fetch("./branch_runs.json").then((r) => r.json());
const DEFAULT_SELECTED_BRANCH = "main";

/**
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
class BranchSelector extends HTMLElement {
  /** @type {ShadowRoot} */
  #dom = null;

  #data = [];

  // SHA of the Head Commit of the selected Branch
  value = "";
  // Name of the Selected branch
  name = "";

  #firedOnload = false;

  constructor() {
    super();
  }

  async connectedCallback() {
    this.#dom = this.attachShadow({ mode: "open" });
    this.render();
    this.#data = await this.getData();
    this.update();
  }

  async getData() {
    const maybeStored = localStorage.getItem("branches");

    if (maybeStored) {
      const stored = JSON.parse(maybeStored);
      const stored_age = new Date() - new Date(stored.date);
      // The branches should be cached for 1 hour
      if (stored_age < 1000 * 60 * 60 * 1) {
        console.log("Using Cached Data");
        return stored.data;
      }
    }

    // We can't use the stored ones >:c - Too old
    const unfiltered_branches = await getGithubPaginatedData(
      octokit,
      "GET /repos/{owner}/{repo}/branches",
      {
        owner: "mozilla-mobile",
        repo: "mozilla-vpn-client",
      },
    );

    let branch_info = [];
    try {
      branch_info = await static_branch_info; // Make Sure this is ready
    } catch (error) {
      console.error(error);
    }

    // Filter the branches we just got from github:
    // if we know the head-sha of the breanch does not have
    // a taskcluster run (because too told), drop it.
    const data = unfiltered_branches.filter((branch) => {
      const branch_metadata = branch_info[branch.commit.sha];
      if (!branch_metadata) {
        // We don't know anything about that branch, add it to final list
        return true;
      }
      if (branch_metadata.name === "main") {
        // It's okay if we did not have valid info about main
        // on build time, as gh pages task could have been faster
        // as the taskcluster-wasm task
        return true;
      }
      // It's ok if it has a running or completed task attached.
      return branch_metadata.task_status == "ok";
    });

    localStorage.setItem(
      "branches",
      JSON.stringify({
        date: new Date(),
        data,
      }),
    );

    return data;
  }

  render() {
    const urlParams = new URLSearchParams(window.location.search);
    const selectedBranch = urlParams.get("branch") || DEFAULT_SELECTED_BRANCH;


    if (this.#dom.innerHTML != "") {
      this.update();
    }
    this.#dom.innerHTML = `
    <style>
        :root {
          --max-height: 300px;
      }
    </style>
    <fluent-combobox id="selector" autocomplete="both" value="${selectedBranch}"></fluent-combobox>`;

    const selector = this.#dom.querySelector("#selector");
    selector.addEventListener("change", (e) => {
      // Forward the event to the parent element;
      this.name = selector.value;
      const option = Array.from(selector.querySelectorAll(`fluent-option`))
        .find(
          (e) => e.innerText == this.name,
        );
      this.value = option.value;
      this.dispatchEvent(new CustomEvent("change", e));
    });
  }
  update() {
    const selector = this.#dom.querySelector("#selector");
    selector.innerHTML = `
            ${this.#data.map((e) => {
      return `<fluent-option value="${e.commit.sha}">${e.name}</fluent-option>`;
    }).join("")
      }
        `;

    if (!this.#firedOnload) {
      requestIdleCallback(() => {
        const url = new URL(window.location);
        let name = url.searchParams.get("branch");
        if (!name) {
          name = "main";
        }
        const option = Array.from(selector.querySelectorAll(`fluent-option`))
          .find(
            (e) => e.innerText == name,
          );
        if (option) {
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
