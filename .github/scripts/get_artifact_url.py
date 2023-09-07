#!/usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/. */


##
#
# Prints a link to a taskcluster artifact 
#



import argparse
import requests
rootURL = "https://firefox-ci-tc.services.mozilla.com"
import taskcluster
index = taskcluster.Index({
  'rootUrl': 'https://firefox-ci-tc.services.mozilla.com',
})

queue = taskcluster.Queue({
     'rootUrl': 'https://firefox-ci-tc.services.mozilla.com',
})

decisionTask = "Md73y6peTHWj2Ubirzvahg"


def main():
    parser = argparse.ArgumentParser(
        description="Fetch a taskcluster artifact"
    )

    parser.add_argument("--decisionTask", dest="decisionTask", action="store", help="Decision Task Task ID")
    parser.add_argument("--task", dest="taskName", action="store", help="Name Of the Task")
    parser.add_argument("--artifact", dest="artifactName", action="store", help="Name Of the Artifact")
    args = parser.parse_args()
    taskgroup=[]
    try:
        taskgroup = queue.listTaskGroup(args.decisionTask)
    except:
        print(f"Failed to get Taskgroup for {args.decisionTask}")
        exit(1)
    tasks = taskgroup["tasks"]
    task = 0
    for t in tasks:
        if t["task"]["metadata"]["name"] == args.taskName:
            task = t
    if task == 0:
        print(f"Failed to get task {args.taskName} in group {args.decisionTask}")
        exit(1)
    if task["status"]["state"] != "completed":
        print(f"expected task {args.taskName} to be completed is: {task['status']['state']}")
        exit(1)
    runID = len(task["status"]["runs"])-1
    taskID = task["status"]["taskId"]
    queue.listArtifacts(taskID,runID)
    print(f"{rootURL}/api/queue/v1/task/{taskID}/runs/{runID}/artifacts/{args.artifactName}")
if __name__ == "__main__":
    main()
