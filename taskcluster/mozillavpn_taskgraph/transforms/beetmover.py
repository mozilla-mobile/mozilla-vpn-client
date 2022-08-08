# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.


import os.path

from taskgraph.transforms.base import TransformSequence
from taskgraph.util.schema import resolve_keyed_by

transforms = TransformSequence()


@transforms.add
def add_beetmover_worker_config(config, tasks):
    for task in tasks:
        app_name = "vpn"
        worker_type = task["worker-type"]
        task_name = task["name"]
        task_label = f"{worker_type}-{task_name}"
        l3_relpro = (
            config.params["level"] == "3"
            and config.params["tasks_for"] in task["run-on-tasks-for"]
        )
        bucket = "release" if l3_relpro else "dep"
        build_id = config.params["moz_build_date"]
        build_type = task["attributes"]["build-type"]
        build_os = os.path.dirname(build_type)
        app_version = config.params["version"]
        candidates_path = os.path.join(
            "pub", app_name, "candidates", app_version, build_id, build_os
        )
        destination_paths = [candidates_path]
        archive_url = (
            "https://ftp.mozilla.org/" if l3_relpro else "https://ftp.stage.mozaws.net/"
        )
        task_description = f"This {worker_type} task will upload a {build_os} release candidate for v{app_version} to {archive_url}{candidates_path}/"
        branch = config.params["head_ref"]

        def get_artifact_path(path):
            # iscript turns .tar.gz files into signed .pkg files
            if build_os == "macos":
                return path.replace(".tar.gz", ".pkg")
            return path

        upstream_artifacts = [
            {
                **upstream_artifact,
                "paths": [
                    get_artifact_path(path) for path in upstream_artifact["paths"]
                ],
            }
            for upstream_artifact in task["worker"]["upstream-artifacts"]
        ]

        artifact_map = []
        for artifact in upstream_artifacts:
            artifact_map.append(
                {
                    "taskId": artifact["taskId"],
                    "paths": {
                        path: {
                            "destinations": [
                                os.path.join(
                                    destination_path,
                                    os.path.basename(path),
                                )
                                for destination_path in destination_paths
                            ]
                        }
                        for path in artifact["paths"]
                    },
                }
            )

        worker = {
            "upstream-artifacts": upstream_artifacts,
            "bucket": bucket,
            "action": "push-to-candidates",
            "release-properties": {
                "app-name": app_name,
                "app-version": app_version,
                "branch": branch,
                "build-id": build_id,
                "platform": build_type,
            },
            "artifact-map": artifact_map,
        }
        task_def = {
            "label": task_label,
            "name": task_label,
            "description": task_description,
            "dependencies": task["dependencies"],
            "worker-type": worker_type,
            "worker": worker,
            "attributes": task["attributes"],
            "run-on-tasks-for": task["run-on-tasks-for"],
        }
        yield task_def
