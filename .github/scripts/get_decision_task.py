#!/usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import argparse
import requests
import time
import os

verbose = False

def maybePrint(s):
    if not verbose:
        return
    print(s)

# Get's a Github check with {check_name}, on a commit {ref}
def get_run(ref,check_name,gh_token, repo="mozilla-vpn-client", OWNER="mozilla-mobile"):
    if len(ref) == 0:
        print("NO REF PROVIDED")
        return
    CHECKS_ENDPOINT = f"https://api.github.com/repos/{OWNER}/{repo}/commits/{ref}/check-runs?check_name={check_name}"
    github_api_headers = {
        "Accept": "application/vnd.github+json",
        "X-GitHub-Api-Version" :"2022-11-28",
    }
    if(gh_token):
        github_api_headers["Authorization"]=f"Bearer {gh_token}"
    else:
        maybePrint("Warning! No GH token")
    r = requests.get(CHECKS_ENDPOINT, headers=github_api_headers)
    return r.json()

DEFAULT_CHECK_NAME="Decision Task (pull-request)"
def wait_for_finished_check(ref, gh_token,check_name=DEFAULT_CHECK_NAME,timeout=30):
    if len(ref) == 0:
        print("NO REF PROVIDED")
        return
    while True:
        # Sleep a bit not to burn through out gh-api rate limit
        # This task is going to run probably before the decision task, 
        # so we need to wait anyway. 
        maybePrint(f"Wait {timeout}s")
        time.sleep(int(timeout))
        run = get_run(ref,check_name=check_name,gh_token=gh_token)
        try:
            if run['total_count'] == 0:
                maybePrint(f"Total Count is 0")
                continue
            status = run["check_runs"][0]["status"]
            if status != "completed":
                maybePrint(f"Not completed is: {status}")
                continue
            maybePrint(f"Run: {run['check_runs'][0]}")
            details_url = run["check_runs"][0]["details_url"]
            return details_url.split("/")[-1]
        except:
            print(run)
            return ""

def main():
    parser = argparse.ArgumentParser(
        description="Get a Taskcluster-Task ID for a github ref"
    )

    parser.add_argument("--gh-token", 
                        dest="token", 
                        action="store", 
                        help="Github API TOKEN", 
                        default=os.environ.get("GITHUB_TOKEN",""))
    parser.add_argument("--task-name", 
                        dest="taskName", 
                        action="store", 
                        help="Name Of the Task", 
                        default=DEFAULT_CHECK_NAME)
    parser.add_argument("--ref", 
                        dest="ref", 
                        action="store", 
                        help="Github Refrence", 
                        default=os.environ.get("GITHUB_REF",""))
    parser.add_argument("--timeout", 
                        dest="timeout", 
                        action="store", 
                        help="Timout Between api calls in seconds", 
                        default=30)
    parser.add_argument("--verbose", 
                        dest="verbose", 
                        action="store_true", 
                        help="Be chatty" )
    args = parser.parse_args()
    global verbose 
    verbose = bool(args.verbose)
    maybePrint(f"Getting {args.taskName} for ref {args.ref}")
    id = wait_for_finished_check(args.ref,args.token,args.taskName,int(args.timeout))
    print(f"taskID={id}")
if __name__ == "__main__":
    main()
