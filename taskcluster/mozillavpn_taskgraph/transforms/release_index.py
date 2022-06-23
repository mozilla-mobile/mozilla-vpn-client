# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.


from taskgraph.transforms.base import TransformSequence

transforms = TransformSequence()

_GIT_REFS_HEADS_PREFIX = "refs/heads/"

BRANCH_INDEX_ROUTES = (
    "index.mozillavpn.v2.mozilla-vpn-client.branch.{branch}.latest.{task_type}.{name}",
)


@transforms.add
def add_branch_index(config, tasks):
    for task in tasks:
        task_type = task.pop("add-branch-index", None)
        if task_type and int(config.params["level"]) == 3:
            name = task["name"].split("/")[0]
            branch = config.params["head_ref"]
            if branch.startswith(_GIT_REFS_HEADS_PREFIX):
                branch = branch[len(_GIT_REFS_HEADS_PREFIX):]

            task.setdefault("routes", [])
            for route in BRANCH_INDEX_ROUTES:
                task["routes"].append(route.format(
                    branch=branch,
                    task_type=task_type,
                    name=name,
                ))
        yield task
