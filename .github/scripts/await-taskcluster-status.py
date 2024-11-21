#!/usr/bin/env python3
import argparse
import requests
import json
import os
import sys
import time

def get_job_status(taskid, api):
    # Request the job status from taskcluster
    r = requests.post(api, json={"taskIds": [taskid]})
    if r.status_code != 200:
        r.raise_for_status()

    # We only requested one job status, so return it.
    return r.json()["statuses"]

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Await a external check to complete")
    parser.add_argument("taskid", metavar="TASKID", type=str, action="store",
                        help="Task ID to wait for")
    parser.add_argument("-i", "--interval", metavar="SEC", type=int, action="store", default=15,
                        help="Interval between retries")
    parser.add_argument("-t", "--timeout", metavar="SEC", type=int, action="store", default=300,
                        help="Task timeout in seconds")
    parser.add_argument("-a", "--api", metavar="URL", type=str, action="store",
                        default='https://firefox-ci-tc.services.mozilla.com/api/queue/v1/tasks/status',
                        help="Taskgraph status API")
    args = parser.parse_args()

    expiration = time.monotonic() + args.timeout
    while time.monotonic() < expiration:
        try:
            js = get_job_status(args.taskid, args.api)
            if len(js) > 0:
                state = js[0]["status"]["state"]
            else:
                state = "not found"
        except Exception as e:
            state = e
        
        if state == 'completed':
            print(f"Task {args.taskid} - {state} - exiting")
            sys.exit(0)

        # Wait and try again        
        maxsleep = expiration - time.monotonic()
        if maxsleep > args.interval:
            maxsleep = args.interval

        print(f"Task {args.taskid} - {state} - retrying in {maxsleep}")
        time.sleep(maxsleep)

    # Otherwise a timeout occurred.
    sys.exit(1)
