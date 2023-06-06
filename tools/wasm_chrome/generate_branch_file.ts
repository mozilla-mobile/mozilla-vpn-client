/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/** Generate Wasm-Chrome Hydration
 *
 * This is a deno script.
 * usage: deno -a generate_branch_file -T <github_api_token>
 *
 * This will generate a file containing current sha->taskClusterID mappings, to save
 * api calls to github and also allow us to filter :)
 */
import { parse } from "https://deno.land/std/flags/mod.ts";
import { Octokit } from "https://cdn.skypack.dev/@octokit/core";
import { getGithubPaginatedData } from "./helpers.mjs";

interface Commit {
  sha: string;
}
interface GhCheck {
  name: string;
}
interface Branch {
  commit: Commit;
  name: string;
}
enum TaskStatus {
  ok = "ok",
  noRun = "noGithubRunInfo", // There was never a wasm task scheudeled for this branch
  notFound = "notFound",
  error = "error",
  tooOld = "tooOld",
  unknown = "unknown",
}
interface StatusResult {
  sha: string;
  name: string;
  task_id: string;
  task_status: TaskStatus;
}

interface TaskClusterStatus {
  error: string;
  code: string;
  status: {
    taskId: string;
    state: string;
  };
}

async function getTaskStatusOf(task_id: string): Promise<TaskStatus> {
  let task_status: TaskClusterStatus;
  try {
    task_status = await fetch(
      `${TASKCLUSTER_INSTANCE}/api/queue/v1/task/${task_id}/status`,
    ).then((r) => r.json());
  } catch (_) {
    return TaskStatus.unknown;
  }
  if (
    task_status.error == "Not found" || task_status.code == "ResourceNotFound"
  ) {
    return TaskStatus.notFound;
  }
  if (task_status.status.state == "running") {
    return TaskStatus.ok;
  }
  if (task_status.status.state != "completed") {
    return TaskStatus.error;
  }
  return TaskStatus.ok;
}

const { T } = parse(Deno.args);

console.log(parse(Deno.args));

const TASKCLUSTER_INSTANCE = "https://firefox-ci-tc.services.mozilla.com";
const TASKCLUSTER_TASK_NAME = "build-wasm/opt";

if (T == "") {
  console.error("No Github Key - Exit");
  console.error("Run with -GH_KEY <your key>");

}

const octokit = new Octokit({
  auth: T,
});

const rate_limit_response = await octokit.request("GET /rate_limit", {});
if (rate_limit_response.status != 200) {
  console.error(rate_limit_response);
  Deno.exit(-1);
} else {
  console.log(`Rate limit status: ${JSON.stringify(rate_limit_response.data.rate, null, 2)}`);
}

// Get Branch data
let branches: Branch[] = await getGithubPaginatedData(
  octokit,
  "GET /repos/{owner}/{repo}/branches",
  {
    owner: "mozilla-mobile",
    repo: "mozilla-vpn-client",
  }
);

const out: Record<string, {
  name: string;
  sha: string;
  task_status: TaskStatus;
  task_id: string;
}> = {};

const jobs = branches.map(async (branch: Branch) => {
  const sha = branch.commit.sha;
  const name = branch.name;

  const result = {
    name,
    sha,
    task_status: TaskStatus.unknown,
    task_id: "",
  };

  // Get All Runs For the Branch
  const response = await octokit.request(
    "GET /repos/{owner}/{repo}/commits/{ref}/check-runs",
    {
      owner: "mozilla-mobile",
      repo: "mozilla-vpn-client",
      ref: sha,
      per_page: 100,
    },
  );
  const checks = response.data.check_runs;
  const wasm_run = checks.find((e: GhCheck) => e.name == TASKCLUSTER_TASK_NAME);
  if (!wasm_run) {
    result.task_status = TaskStatus.noRun;
    out[sha] = result;
    console.log(`${result.task_status} \t \t -> ${name}`);
    return;
  }
  const task_url = wasm_run.details_url;
  result.task_id = task_url.split("/").at(-1);
  result.task_status = await getTaskStatusOf(result.task_id);
  console.log(`${result.task_status} \t \t -> ${name}`);
  out[sha] = result;
});

await Promise.all(jobs);
const encoder = new TextEncoder();
const data = encoder.encode(JSON.stringify(out));
Deno.writeFileSync("branch_runs.json", data);
