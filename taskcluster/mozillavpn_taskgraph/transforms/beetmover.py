# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.


import os.path

from taskgraph.transforms.base import TransformSequence

transforms = TransformSequence()


@transforms.add
def add_addons_release_artifacts(config, tasks):
    for task in tasks:
        if (
            task["attributes"]["build-type"] == "addons/opt"
            and task["name"] == "addons-bundle"
        ):
            addons = set(
                name
                for name in os.listdir("addons")
                if os.path.isdir(os.path.join("addons", name))
                and os.path.isfile(os.path.join("addons", name, "manifest.json"))
            )
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
    build_id = config.params["moz_build_date"]
    build_type_os = {
        "macos/opt": "mac",
        "windows/opt": "windows",
        "android/x86": "android",
        "android/x64": "android",
        "android/armv7": "android",
        "android/arm64-v8a": "android",
    }

    if config.params["version"]:
        app_version = config.params["version"]
    elif "releases" in config.params["head_ref"]:
        app_version = config.params["head_ref"].split("/")[-1]
    else:
        app_version = ""  # addons are not versioned

    is_production = (
        config.params["level"] == "3" and config.params["tasks_for"] == "action"
    )
    bucket = "release" if is_production else "dep"
    archive_url = (
        "https://ftp.mozilla.org/" if is_production else "https://ftp.stage.mozaws.net/"
    )

    for task in tasks:
        worker_type = task["worker-type"]
        build_type = task["attributes"]["build-type"]
        build_os = build_type_os.get(build_type)
        shipping_phase = config.params.get("shipping_phase", "")

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
        elif shipping_phase == "promote-client":
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
        elif shipping_phase == "ship-client":
            destination_paths.append(
                os.path.join(
                    "pub",
                    "vpn",
                    "releases",
                    app_version,
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

        dest = (
            f"{archive_url}{destination_paths[0]}" if destination_paths else archive_url
        )
        if build_type == "addons/opt":
            task_description = (
                f"This {worker_type} task will upload the {task['name']} to {dest}/"
            )
        elif shipping_phase == "ship-client":
            task_description = f"This {worker_type} task will copy build {build_id} from candidates to releases"
        else:
            task_description = f"This {worker_type} task will upload a {build_os} release candidate for v{app_version} to {dest}/"

        if not shipping_phase or shipping_phase.startswith("promote"):
            action = "push-to-candidates"
        elif shipping_phase == "ship-addons":
            action = "direct-push-to-bucket"
        elif shipping_phase == "ship-client":
            action = "push-to-releases"
        else:
            raise Exception(f"Invalid shipping_phase `{shipping_phase}`")

        extra = {
            "release_destinations": [
                f"{archive_url}{dest}/" for dest in destination_paths
            ]
        }
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
            "extra": extra,
        }
        yield task_def
