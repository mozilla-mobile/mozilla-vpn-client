# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.


import os.path

from taskgraph.transforms.base import TransformSequence

transforms = TransformSequence()


@transforms.add
def add_beetmover_worker_config(config, tasks):
    for task in tasks:
        app_name = "vpn"
        worker_type = task["worker-type"]
        is_relpro = (
            config.params["level"] == "3"
            and config.params["tasks_for"] in task["run-on-tasks-for"]
        )
        bucket = "release" if is_relpro else "dep"
        build_id = config.params["moz_build_date"]
        build_type = task["attributes"]["build-type"]
        build_os = os.path.dirname(build_type)
        app_version = config.params["version"]
        candidates_path = os.path.join(
            "pub", app_name, "candidates", app_version, build_id, build_os
        )
        destination_paths = [candidates_path]
        archive_url = (
            "https://ftp.mozilla.org/" if is_relpro else "https://ftp.stage.mozaws.net/"
        )
        task_description = f"This {worker_type} task will upload a {build_os} release candidate for v{app_version} to {archive_url}{candidates_path}/"
        branch = config.params["head_ref"]

        upstream_artifacts = []
        for dep in task["dependencies"]:
            upstream_artifacts.append(
                {
                    "taskId": {"task-reference": f"<{dep}>"},
                    "taskType": "scriptworker",
                    "paths": [
                        release_artifact["name"]
                        for release_artifact in task["attributes"]["release-artifacts"]
                    ],
                }
            )

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
            "name": task["name"],
            "description": task_description,
            "dependencies": task["dependencies"],
            "worker-type": worker_type,
            "worker": worker,
            "attributes": task["attributes"],
            "run-on-tasks-for": task["run-on-tasks-for"],
        }
        yield task_def
