#!/usr/bin/env python3
import argparse
import requests
import json
import os
import sys

GITHUB_TOKEN = os.environ.get('GITHUB_TOKEN')

def fetch(url, version='2022-11-28', timeout=None):
    headers = {
        'Accept': 'application/vnd.github+json',
        'X-GitHub-Api-Version': version,
    }
    if GITHUB_TOKEN is not None:
        headers['Authorization'] = f"Bearer {GITHUB_TOKEN}"

    r = requests.get(url, headers)
    if r.status_code != 200:
        r.raise_for_status()
    return r.json()

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Await a external check to complete")
    parser.add_argument("ref", metavar="REF", type=str, action="store",
                        help="Github reference fetch checks for")
    parser.add_argument("name", metavar="NAME", type=str, action="store",
                        help="Name of the check to wait for")
    parser.add_argument("-a", "--app", metavar="SUITE", type=str, action="store",
                        help="App name to fetch checks from")
    args = parser.parse_args()

    # Find the URL to fetch checks from.
    checks_url = None
    if args.app:
        js = fetch(f"https://api.github.com/repos/mozilla-mobile/mozilla-vpn-client/commits/{args.ref}/check-suites")
        for suite in js['check_suites']:
            if suite["app"]["name"] == args.app:
                checks_url = suite["check_runs_url"]
        if not checks_url:
            raise KeyError(f"No suite {args.app} found for ${args.ref}")
    else:
        checks_url = f"https://api.github.com/repos/mozilla-mobile/mozilla-vpn-client/commits/{args.ref}/check-runs"

    # Fetch the JSON for the desired check.
    js = fetch(checks_url)
    for check in js['check_runs']:
        if check["name"] == args.name:
            json.dump(check, sys.stdout)

    print(f"DEBUG: {checks_url}")
