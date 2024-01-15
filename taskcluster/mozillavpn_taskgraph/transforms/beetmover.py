# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.


import os.path

from taskgraph.transforms.base import TransformSequence
from taskgraph.transforms.task import task_description_schema
from taskgraph.util.schema import Schema
from voluptuous import Extra, Optional, Required

transforms = TransformSequence()


beetmover_schema = Schema(
    {
        Required("beetmover-action"): str,
        Required("attributes"): {
            Required("build-type"): str,
            Required("release-artifacts"): [dict],
            Extra: object,
        },
        Required("dependencies"): task_description_schema["dependencies"],
        Required("name"): str,
        Required("run-on-tasks-for"): task_description_schema["run-on-tasks-for"],
        Required("worker-type"): task_description_schema["worker-type"],
        Optional("task-from"): task_description_schema["task-from"],
    }
)


@transforms.add
def remove_worker(config, tasks):
    """The `release_artifacts` transforms add a key to 'worker' which we don't
    use here, remove it."""
    for task in tasks:
        if "worker" in task:
            del task["worker"]
        yield task


transforms.add_validate(beetmover_schema)


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
    short_phase = config.kind[len("beetmover-"):]

    for task in tasks:
        worker_type = task["worker-type"]
        build_type = task["attributes"]["build-type"]
        build_os = build_type_os.get(build_type)
        phase = f"{short_phase}-{'addons' if task['name'].startswith('addons') else 'client'}"

        upstream_artifacts = []
        for dep in task["dependencies"]:
            if dep not in ("build", "signing"):
                continue
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

        worker = {
            "action": task["beetmover-action"],
            "artifact-map": [],
            "bucket": bucket,
            "build-number": int(build_id),
            "release-properties": {
                "app-name": "vpn",
                "app-version": app_version,
                "branch": config.params["head_ref"],
                "build-id": build_id,
                "platform": build_type,
            },
            "upstream-artifacts": upstream_artifacts
        }

        destination_paths = []

        if task["name"].startswith("addons"):
            destination_paths.append(
                os.path.join(
                    "pub",
                    "vpn",
                    "addons",
                    "releases" if short_phase == "ship" else "candidates",
                    build_id,
                )
            )
            if phase == "ship-addons":
                destination_paths.append(
                    os.path.join(
                        "pub",
                        "vpn",
                        "addons",
                        "releases",
                        "latest",
                    )
                )
        elif phase == "promote-client":
            assert build_os
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
        elif phase == "ship-client":
            assert build_os
            destination_paths.append(
                os.path.join(
                    "pub",
                    "vpn",
                    "releases",
                    app_version,
                    build_os,
                )
            )

        for artifact in upstream_artifacts:
            worker["artifact-map"].append(
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
            "shipping-phase": phase,
        }

        dest = (
            f"{archive_url}{destination_paths[0]}"
            if destination_paths
            else archive_url
        )
        if build_type == "addons/opt":
            task_description = (
                f"This {worker_type} task will upload the {task['name']} to {dest}/"
            )
        elif phase == "ship-client":
            task_description = f"This {worker_type} task will copy the v{app_version} build {build_id} candidate to releases"
        else:
            task_description = f"This {worker_type} task will upload a {build_os} release candidate for v{app_version} to {dest}/"

        extra = {
            "release_destinations": [
                f"{archive_url}{dest}/" for dest in destination_paths
            ]
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
