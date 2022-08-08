# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
"""
Apply some defaults and minor modifications to the jobs defined in the build
kind.
"""
from taskgraph.transforms.base import TransformSequence
from taskgraph.util.schema import Schema
from voluptuous import Optional, Required, Extra


transforms = TransformSequence()

build_schema = Schema(
    {
        Required("name"): str,
        Optional("requires-level"): int,
        Extra: object,
    }
)

transforms.add_validate(build_schema)


# Deletes a task when the current level is < then the required level
@transforms.add
def checkRequiredLevel(config, tasks):
    for task in tasks:
        if "requires-level" in task:
            requiredLevel = int(task.pop("requires-level"))
            currentLevel = int(config.params["level"])
            if requiredLevel <= currentLevel:
                yield task
        else:
            yield task


# Add build-type info to the Task. Defaults to taskname
@transforms.add
def add_variant_config(config, tasks):
    for task in tasks:
        attributes = task.setdefault("attributes", {})
        if not attributes.get("build-type"):
            attributes["build-type"] = task["name"]
        yield task
