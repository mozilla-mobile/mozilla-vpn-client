# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.


from taskgraph.transforms.base import TransformSequence
transforms = TransformSequence()

_GIT_REFS_HEADS_PREFIX = "refs/heads/"


@transforms.add
def add_branch_index(config, tasks):
    for task in tasks:
        if "add-branch-index" in task:
            release_index = task.pop("add-branch-index")
            if release_index and int(config.params["level"]) == 3:
                name = task['name'].split("/")[0]
                git_branch = config.params["head_ref"]
                if git_branch.startswith(_GIT_REFS_HEADS_PREFIX):
                    git_branch = git_branch[len(_GIT_REFS_HEADS_PREFIX):]
                route = f"index.mozillavpn.v2.mozilla-vpn-client.branch.{git_branch}.latest.{name}"
                task.setdefault("routes", []).append(route)
        yield task
