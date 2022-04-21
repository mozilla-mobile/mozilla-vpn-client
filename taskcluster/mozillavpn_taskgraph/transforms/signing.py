# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
"""
Apply some defaults and minor modifications to the jobs defined in the build
kind.
"""

from taskgraph.transforms.base import TransformSequence
from taskgraph.util.schema import resolve_keyed_by


transforms = TransformSequence()

PRODUCTION_SIGNING_BUILD_TYPES = [
    "android-x64/release",
    "android-x86/release",
    "android-arm64/release",
    "android-armv7/release",
    # "macos/opt",
]

SIGNING_BUILD_TYPES = PRODUCTION_SIGNING_BUILD_TYPES + [
    # Note: it appears we don't have infra for debug sign
    # contact releng if you need it :)
    #    "android-debug",
    #
]


@transforms.add
def set_run_on_tasks_for(config, tasks):
    for task in tasks:
        if task["attributes"]["build-type"] in SIGNING_BUILD_TYPES:
            task["run-on-tasks-for"] = ["github-push"]
        yield task


@transforms.add
def resolve_keys(config, tasks):
    for task in tasks:
        for key in ("signing-format",):
            resolve_keyed_by(
                task,
                key,
                item_name=task["name"],
                **{
                    "build-type": task["attributes"]["build-type"],
                    "level": config.params["level"],
                    "tasks-for": config.params["tasks_for"],
                }
            )
        yield task


@transforms.add
def set_worker_type(config, tasks):
    for task in tasks:
        worker_type = "dep-signing"
        if (
            str(config.params["level"]) == "3"
            and task["attributes"]["build-type"] in PRODUCTION_SIGNING_BUILD_TYPES
        ):
            worker_type = "signing"
        task["worker-type"] = worker_type
        yield task


@transforms.add
def set_signing_type(config, tasks):
    for task in tasks:
        signing_type = "dep-signing"
        if str(config.params["level"]) == "3":
            if task["attributes"]["build-type"] in PRODUCTION_SIGNING_BUILD_TYPES:
                signing_type = "release-signing"
        task.setdefault("worker", {})["signing-type"] = signing_type
        yield task


@transforms.add
def set_signing_attributes(config, tasks):
    for task in tasks:
        task["attributes"]["signed"] = True
        yield task


@transforms.add
def set_signing_format(config, tasks):
    for task in tasks:
        signing_format = task.pop("signing-format")
        for upstream_artifact in task["worker"]["upstream-artifacts"]:
            upstream_artifact["formats"] = [signing_format]
        yield task
