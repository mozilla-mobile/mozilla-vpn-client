# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from taskgraph.transforms.base import TransformSequence
from taskgraph.util.schema import resolve_keyed_by

transforms = TransformSequence()

@transforms.add
def resolve_keys(config, tasks):
    for task in tasks:
        for key in ("dependencies", "attributes.build-type"):
            resolve_keyed_by(task, key, item_name=task["name"], **{
                    "level": config.params["level"],
            })
        yield task

@transforms.add
def resolve_from_dependency(config, tasks):
    for task in tasks:
        if "from-dependency" in task["fetches"]:
            depname = list(task["dependencies"].keys())[0]
            task["fetches"][depname] = task["fetches"].pop("from-dependency")

        yield task
