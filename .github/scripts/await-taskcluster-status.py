#!/usr/bin/env python
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
            if len(js) == 0:
                raise Exception("not found")
            status = js[0]["status"]
        except Exception as e:
            status = {
                "taskId": args.taskid,
                "state": str(e),
                "runs": [],
            }
        
        if status["state"] == 'completed':
            print(f'Task {args.taskid} - {status["state"]} - exiting', file=sys.stderr)
            print(f'{json.dumps(status, indent=3)}')
            sys.exit(0)

        # Wait and try again        
        maxsleep = expiration - time.monotonic()
        if maxsleep > args.interval:
            maxsleep = args.interval

        print(f'Task {args.taskid} - {status["state"]} - retrying in {maxsleep}', file=sys.stderr)
        time.sleep(maxsleep)

    # Otherwise a timeout occurred.
    sys.exit(1)
