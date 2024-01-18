# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
"""
Build upstream_artifacts from dependencies, conforming to the scriptworker
payload.
"""

from taskgraph.transforms.base import TransformSequence


upstream_artifacts = TransformSequence()


@upstream_artifacts.add
def build_upstream_artifacts(config, tasks):
    for task in tasks:
        worker_definition = {
            "upstream-artifacts": [],
        }

        for label, dep in config.kind_dependencies_tasks.items():
            if label not in task["dependencies"].values():
                continue

            paths = sorted(
                artifact["name"]
                for artifact in dep.attributes.get("release-artifacts", [])
            )

            if paths:
                worker_definition["upstream-artifacts"] = [
                    {
                        "taskId": {"task-reference": f"<{dep.kind}>"},
                        "taskType": dep.kind,
                        "paths": paths,
                    }
                ]

        task.setdefault("worker", {}).update(worker_definition)
        yield task

@upstream_artifacts.add
def add_authenticode_comment(config, tasks):
    for task in tasks: 
        artifacts = task["worker"]["upstream-artifacts"]
        for artifact_dict in artifacts:
            authenticode_comment = artifact_dict.get("authenticode_comment")
            if not authenticode_comment and any(path.endswith(".msi") for path in artifact_dict["paths"]):
                artifact_dict["authenticode_comment"]="Mozilla VPN Client installer"
        yield task 
