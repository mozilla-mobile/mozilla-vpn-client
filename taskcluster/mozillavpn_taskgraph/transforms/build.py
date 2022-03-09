# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
"""
Apply some defaults and minor modifications to the jobs defined in the build
kind.
"""

from taskgraph.transforms.base import TransformSequence
from taskgraph.transforms.task import task_description_schema
from taskgraph.util.schema import Schema
from voluptuous import Optional, Required, Any


transforms = TransformSequence()

build_schema = Schema(
    {
        Required("name"): str,
        Required("description"): task_description_schema["description"],
        Required("treeherder"): task_description_schema["treeherder"],
        Required("worker-type"): task_description_schema["worker-type"],
        Optional("scopes"): task_description_schema["scopes"],
        Optional("job-from"): task_description_schema["job-from"],
        Required("worker"): object,
        Required("run"): {str: Any(str, bool)},
        Optional("requires-level"): int,
        Optional("release-artifacts"): [str],
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
def add_artifacts(config, tasks):
    for task in tasks:
        artifacts = task.setdefault("worker", {}).setdefault("artifacts", [])

        # Android artifacts
        if "release-artifacts" in task:
            for filename in task.pop("release-artifacts"):
                artifacts.append(
                    {
                        "type": "file",
                        "name": f"public/build/{filename}",
                        "path": f"/builds/worker/artifacts/{filename}",
                    }
                )

        task["attributes"]["release-artifacts"] = artifacts
        yield task
