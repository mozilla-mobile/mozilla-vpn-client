# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.


import os.path

from taskgraph.transforms.base import TransformSequence

transforms = TransformSequence()


@transforms.add
def add_addons_release_artifacts(config, tasks):
    for task in tasks:
        if task["attributes"]["build-type"] == "addons/opt" and task["name"] == "addons-bundle":
            addons = set(os.listdir("addons"))
            addons.remove("examples")
            addons.remove("deprecated")
            for addon in addons:
                task["attributes"]["release-artifacts"].append(
                    {
                        "type": "file",
                        "name": f"public/build/{addon}.rcc",
                        "path": f"/builds/worker/artifacts/{addon}.rcc",
                    }
                )
        yield task


@transforms.add
def add_beetmover_worker_config(config, tasks):
    for task in tasks:
        worker_type = task["worker-type"]
        is_relpro = (
            config.params["level"] == "3"
            and config.params["tasks_for"] in task["run-on-tasks-for"]
        )
        bucket = "release" if is_relpro else "dep"
        build_id = config.params["moz_build_date"]
        build_type = task["attributes"]["build-type"]
        build_type_os = {
            'macos/opt': 'mac',
            'windows/opt': 'windows',
        }
        build_os = build_type_os.get(build_type)
        shipping_phase = config.params.get("shipping_phase", "")

        if config.params["version"]:
            app_version = config.params["version"]
        elif "releases" in config.params["head_ref"]:
            app_version = config.params["head_ref"].split("/")[-1]
        else:
            app_version = ""  # addons are not versioned

        destination_paths = []

        if build_type == "addons/opt":
            destination_paths.append(
                os.path.join(
                    "pub",
                    "vpn",
                    "addons",
                    "releases" if shipping_phase.startswith("ship") else "candidates",
                    build_id,
                )
            )
        else:
            destination_paths.append(
                os.path.join(
                    "pub",
                    "vpn",
                    "candidates",
                    f"{app_version}-candidates",
                    f"build{build_id}",
                    build_os,
                )
            )

        if shipping_phase == "ship-addons":
            destination_paths.append(
                os.path.join(
                    "pub",
                    "vpn",
                    "addons",
                    "releases",
                    "latest",
                )
            )

        archive_url = (
            "https://ftp.mozilla.org/" if is_relpro else "https://ftp.stage.mozaws.net/"
        )

        upstream_artifacts = []
        for dep in task["dependencies"]:
            upstream_artifacts.append(
                {
                    "taskId": {"task-reference": f"<{dep}>"},
                    "taskType": dep if dep == "build" else "scriptworker",
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

        attributes = {
            **task["attributes"],
            "shipping-phase": shipping_phase,
        }

        if build_type == "addons/opt":
            task_description = f"This {worker_type} task will upload the {task['name']} to {archive_url}{destination_paths[0]}/"
        elif shipping_phase == "ship-client":
            task_description = f"This {worker_type} task will copy build {build_id} from candidates to releases"
        else:
            task_description = f"This {worker_type} task will upload a {build_os} release candidate for v{app_version} to {archive_url}{destination_paths[0]}/"

        if not shipping_phase or shipping_phase.startswith("promote"):
            action = "push-to-candidates"
        elif shipping_phase == "ship-addons":
            action = "direct-push-to-bucket"
        elif shipping_phase == "ship-client":
            action = "push-to-releases"
        else:
            raise Exception(f"Invalid shipping_phase `{shipping_phase}`")

        worker = {
            "upstream-artifacts": upstream_artifacts,
            "bucket": bucket,
            "action": action,
            "release-properties": {
                "app-name": "vpn",
                "app-version": app_version,
                "branch": config.params["head_ref"],
                "build-id": build_id,
                "platform": build_type,
            },
            "artifact-map": artifact_map,
            "build-number": int(build_id),
        }
        task_def = {
            "name": task["name"],
            "description": task_description,
            "dependencies": task["dependencies"],
            "worker-type": worker_type,
            "worker": worker,
            "attributes": attributes,
            "run-on-tasks-for": task["run-on-tasks-for"],
        }
        yield task_def
