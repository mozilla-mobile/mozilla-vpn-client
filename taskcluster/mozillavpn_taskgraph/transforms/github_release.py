# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import os.path

from taskgraph.transforms.base import TransformSequence

transforms = TransformSequence()

@transforms.add
def build_artifact_map(config, tasks):
    for task in tasks:
        artifact_map = []
        for artifact in task["worker"]["upstream-artifacts"]:
            artifact_map.append(
                {
                    "taskId": artifact["taskId"],
                    "paths": {
                        path: {
                            "destinations": [os.path.basename(path)]
                        }
                        for path in artifact["paths"]
                    },
                }
            )

        task["worker"]["artifact-map"] = artifact_map
        yield task

@transforms.add
def build_parameters(config, tasks):
    for task in tasks:
        worker = task.setdefault("worker", {})
        worker["git-revision"] = config.params["head_rev"]

        for field in (
            "release-body",
            "release-name",
            "git-tag",
        ):
            if field in worker:
                worker[field] = worker[field].format(**config.params)
        yield task
