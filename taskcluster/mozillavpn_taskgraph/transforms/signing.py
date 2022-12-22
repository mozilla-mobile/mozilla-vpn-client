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
    "macos/opt",
    "windows/opt",
    "addons/opt",
]

SIGNING_BUILD_TYPES = PRODUCTION_SIGNING_BUILD_TYPES + [
    # Note: it appears we don't have infra for debug signing
    # android builds. Contact releng if you need it :)
    # "android-debug",
    # "addons/opt",  # TODO: Add addons debug builds? We have the infra to debug sign them.
    "source/vpn",  # Needs testing / validation
]


@transforms.add
def set_run_on_tasks_for(config, tasks):
    for task in tasks:
        if task["attributes"]["build-type"] in SIGNING_BUILD_TYPES:
            task["run-on-tasks-for"] = ["github-push", "github-pull-request"]
        yield task


@transforms.add
def resolve_keys(config, tasks):
    for task in tasks:
        for key in ("signing-format", "treeherder.job-symbol"):
            resolve_keyed_by(
                task,
                key,
                item_name=task["name"],
                **{
                    "build-type": task["attributes"]["build-type"],
                    "level": config.params["level"],
                    "tasks-for": config.params["tasks_for"],
                },
            )
        yield task


@transforms.add
def set_worker_and_signing_type(config, tasks):
    for task in tasks:
        worker_type = signing_type = "dep-signing"
        build_type = task["attributes"]["build-type"]
        if (
            str(config.params["level"]) == "3"
            and build_type in PRODUCTION_SIGNING_BUILD_TYPES
        ):
            signing_type = "release-signing"
            worker_type = "signing"

        if build_type.startswith("macos"):
            worker_type = f"macos-{worker_type}"

        task["worker-type"] = worker_type
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


def script_url(params, path):
    return f"{params['head_repository']}/raw/{params['head_rev']}/{path}"


@transforms.add
def set_mac_behavior(config, tasks):
    for task in tasks:
        if not task["attributes"]["build-type"].startswith("macos"):
            yield task
            continue

        task["worker"]["mac-behavior"] = "mac_notarize_vpn"
        task["worker"]["entitlementsUrl"] = script_url(
            config.params, "taskcluster/scripts/signing/entitlements.xml"
        )
        task["worker"]["loginItemsEntitlementsUrl"] = script_url(
            config.params, "taskcluster/scripts/signing/loginItems-entitlements.xml"
        )
        yield task
