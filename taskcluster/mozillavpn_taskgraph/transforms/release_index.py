# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from voluptuous import Any, Extra, Optional

from taskgraph.transforms.base import TransformSequence
from taskgraph.util.schema import Schema, optionally_keyed_by, resolve_keyed_by

transforms = TransformSequence()

_GIT_REFS_HEADS_PREFIX = "refs/heads/"

BRANCH_INDEX_ROUTES = (
    "index.mozillavpn.v2.mozilla-vpn-client.branch.{branch}.latest.{type}.{name}",
)
RELEASE_INDEX_ROUTES = (
    "index.mozillavpn.v2.mozilla-vpn-client.release.{version}.latest.{type}.{name}",
)


release_index_schema = Schema(
    {
        Optional("add-index-routes"): optionally_keyed_by(
            "build-type", Any(str, {"name": str, "type": str}, None)
        ),
        Extra: object,
    }
)

transforms.add_validate(release_index_schema)


@transforms.add
def resolve_keys(config, tasks):
    for task in tasks:
        resolve_keyed_by(
            task, "add-index-routes", item_name=task["name"], **task.get("attributes", {})
        )
        yield task


@transforms.add
def add_index_routes(config, tasks):
    for task in tasks:
        context = task.pop("add-index-routes", None)
        if not context or int(config.params["level"]) != 3:
            yield task
            continue

        if isinstance(context, str):
            context = {
                "name": task["name"].split("/")[0],
                "type": context,
            }
        routes = []
        if config.params["tasks_for"] == "github-push":
            routes = BRANCH_INDEX_ROUTES
            branch = config.params["head_ref"]
            if branch.startswith(_GIT_REFS_HEADS_PREFIX):
                branch = branch[len(_GIT_REFS_HEADS_PREFIX):]
            branch = branch.replace("/",".") # We can't have "/" in the index -> so releases/2.9.0 needs to be releases.2.9.0
            context["branch"] = branch

        elif config.params["tasks_for"] == "github-release":
            routes = RELEASE_INDEX_ROUTES
            context["version"] = config.params["head_tag"]

        task.setdefault("routes", []).extend(
            [route.format(**context) for route in routes]
        )
        yield task
