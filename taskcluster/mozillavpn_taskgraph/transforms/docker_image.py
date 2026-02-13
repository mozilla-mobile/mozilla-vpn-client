# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
"""
Update task definition using the Docker Hub API
"""
import re
import requests

from taskgraph.transforms.base import TransformSequence

transforms = TransformSequence()

def dockerhub_fetch_digest(repo, tag):
    h = { 'Accept': 'application/json' }
    r = requests.get(f'https://hub.docker.com/v2/namespaces/library/repositories/{repo}/tags/{tag}', headers=h)
    return r.json().get("digest")

@transforms.add
def resolve_tag_digest(config, tasks):
    for task in tasks:
        if not "args" in task:
            continue

        # If we find text of the form: {dockerhub:repository:tag} then use the
        # Docker Hub API to replace it with the hash of the tag.
        dockerhub_regex = re.compile('{dockerhub:[a-z0-9:]+}')
        for key, value in task.get("args", {}).items():
            match = dockerhub_regex.search(value)
            if match:
                dh_args = match.group(0)[1:-1].split(':')
                digest = dockerhub_fetch_digest(dh_args[1], dh_args[2])
                tag = f'{dh_args[1]}:{dh_args[2]}@{dh_digest}'
                if dh_digest:
                    task["args"][key] = value[0:match.start()] + tag + value[match.end():]

        yield task
