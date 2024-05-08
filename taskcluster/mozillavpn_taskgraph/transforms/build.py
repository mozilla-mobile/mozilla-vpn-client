# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
"""
Apply some defaults and minor modifications to the jobs defined in the build
kind.
"""
from taskgraph.transforms.base import TransformSequence 
from taskgraph.util.schema import Schema
from taskgraph.util.schema import resolve_keyed_by
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



@transforms.add
def resolve_keys_scope(config, tasks):
    # Enables the by-level:
    # resolver for the "scopes field"
    # usage:
    # scopes:
    #   by-level:
    #     3: [scope-a,scope-b]
    #     1: [scope-a]
    for task in tasks:
        resolve_keyed_by(
            task,
            "scopes",
            item_name=task["name"],
            **{
                "level": config.params["level"],
            },
        )
        yield task


@transforms.add
def add_build_metadata(config, tasks):
    for task in tasks:
        task["worker"].setdefault("env", {})
        if config.params["pull_request_number"]:
            task["worker"]["env"].update({
                "PULL_REQUEST_NUMBER": str(config.params["pull_request_number"])
            })
        if config.params["owner"]:
            task["worker"]["env"].update({
                "TASK_OWNER": config.params["owner"]
            })
        if config.params["build_date"]:
            task["worker"]["env"].update({
                "BUILD_TIMESTAMP": config.params["build_date"]
            })

        yield task
