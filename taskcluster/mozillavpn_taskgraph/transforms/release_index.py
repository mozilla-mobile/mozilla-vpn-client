# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.


from taskgraph.transforms.base import TransformSequence

transforms = TransformSequence()

_GIT_REFS_HEADS_PREFIX = "refs/heads/"

BRANCH_INDEX_ROUTES = (
    "index.mozillavpn.v2.mozilla-vpn-client.branch.{branch}.latest.{task_type}.{name}",
)
RELEASE_INDEX_ROUTES = (
    "index.mozillavpn.v2.mozilla-vpn-client.release.{version}.latest.{task_type}.{name}",
)


@transforms.add
def add_index_routes(config, tasks):
    for task in tasks:
        task_type = task.pop("add-index-routes", None)
        if not task_type or int(config.params["level"]) != 3:
            yield task
            continue

        routes = []
        context = {
            "name": task["name"].split("/")[0],
            "task_type": task_type,
        }
        if config.params["tasks_for"] == "github-push":
            routes = BRANCH_INDEX_ROUTES
            branch = config.params["head_ref"]
            if branch.startswith(_GIT_REFS_HEADS_PREFIX):
                branch = branch[len(_GIT_REFS_HEADS_PREFIX):]
            context["branch"] = branch

        elif config.params["tasks_for"] == "github-release":
            routes = RELEASE_INDEX_ROUTES
            context["version"] = config.params["head_tag"]

        task.setdefault("routes", []).extend(
            [route.format(**context) for route in routes]
        )
        yield task
