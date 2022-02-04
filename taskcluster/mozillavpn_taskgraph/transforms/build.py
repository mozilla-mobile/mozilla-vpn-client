# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
"""
Apply some defaults and minor modifications to the jobs defined in the build
kind.
"""


import datetime

from taskgraph.transforms.base import TransformSequence

transforms = TransformSequence()



@transforms.add
def checkRequiredLevel(config, tasks):
    for task in tasks:
        if("requiresLevel" in task):
            requiredLevel = int(task.pop("requiresLevel"))
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
        task["attributes"]["apks"] = apks = {}

        if "apk-artifacts" in task:
            artifact_template = task.pop("apk-artifacts")
            for apk_name in artifact_template:
                
                artifacts.append({
                    "type": "file",
                    "name": "public/build/vpn/"+apk_name,
                    "path": "/builds/worker/artifacts/"+apk_name,
                })
                apks[apk_name] = {
                    "name": "public/build/vpn/"+apk_name,
                }

        yield task


