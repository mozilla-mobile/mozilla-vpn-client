# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from datetime import datetime

from taskgraph.transforms.task import payload_builder
from taskgraph.util.schema import taskref_or_string
from voluptuous import Any, Optional, Required


@payload_builder(
    "scriptworker-signing",
    schema={
        # the maximum time to run, in seconds
        Required("max-run-time"): int,
        Required("signing-type"): str,
        # list of artifact URLs for the artifacts that should be signed
        Required("upstream-artifacts"): [
            {
                # taskId of the task with the artifact
                Required("taskId"): taskref_or_string,
                # type of signing task (for CoT)
                Required("taskType"): str,
                # Paths to the artifacts to sign
                Required("paths"): [str],
                # Signing formats to use on each of the paths
                Required("formats"): [str],
                # Only For MSI, optional for the signed Installer
                Optional("authenticode_comment"): str,
            }
        ],
        # behavior for mac iscript
        Optional("mac-behavior"): Any(
            "mac_sign_and_pkg_vpn",
        ),
        Optional("entitlementsUrl"): str,
        Optional("loginItemsEntitlementsUrl"): str,
    },
)
def build_scriptworker_signing_payload(config, task, task_def):
    worker = task["worker"]

    task_def["tags"]["worker-implementation"] = "scriptworker"

    task_def["payload"] = {
        "maxRunTime": worker["max-run-time"],
        "upstreamArtifacts": worker["upstream-artifacts"],
    }

    if worker.get("mac-behavior"):
        task_def["payload"]["behavior"] = worker["mac-behavior"]
        for attr in ("entitlementsUrl", "loginItemsEntitlementsUrl"):
            if attr in worker:
                task_def["payload"][attr] = worker[attr]

    formats = set()
    for artifacts in worker["upstream-artifacts"]:
        formats.update(artifacts["formats"])

    scope_prefix = "project:mozillavpn"
    task_def["scopes"].append(
        "{}:releng:signing:cert:{}".format(scope_prefix, worker["signing-type"])
    )
    task_def["scopes"].extend(
        [f"{scope_prefix}:releng:signing:format:{format}" for format in sorted(formats)]
    )


@payload_builder(
    "scriptworker-pushapk",
    schema={
        Required("upstream-artifacts"): [
            {
                Required("taskId"): taskref_or_string,
                Required("taskType"): str,
                Required("paths"): [str],
            }
        ],
        Required("certificate-alias"): str,
        Required("commit"): bool,
        Required("channel"): str,
        Required("product"): str,
    },
)
def build_push_apk_payload(config, task, task_def):
    worker = task["worker"]
    task_def["tags"]["worker-implementation"] = "scriptworker"
    task_def["payload"] = {
        "certificate_alias": worker["certificate-alias"],
        "commit": worker["commit"],
        "upstreamArtifacts": worker["upstream-artifacts"],
        "channel": worker["channel"],
    }
    scope_prefix = "project:mozillavpn:releng:googleplay:track"
    task_def["scopes"].append(
        "project:mozillavpn:releng:googleplay:product:{}".format(worker["product"])
    )


@payload_builder(
    "scriptworker-beetmover",
    schema={
        Required("bucket"): str,
        Required("action"): str,
        Required("artifact-map"): [
            {
                Required("paths"): {
                    Any(str): {
                        Required("destinations"): [str],
                    },
                },
                Required("taskId"): taskref_or_string,
            }
        ],
        Required("release-properties"): {
            Required("app-name"): str,
            Required("app-version"): str,
            Required("branch"): str,
            Required("build-id"): str,
            Optional("hash-type"): str,
            Optional("platform"): str,
        },
        Required("upstream-artifacts"): [
            {
                Required("taskId"): taskref_or_string,
                Required("taskType"): str,
                Required("paths"): [str],
            }
        ],
        Optional("build-number"): int,
    },
)
def build_scriptworker_beetmover_payload(config, task, task_def):
    task_def["tags"]["worker-implementation"] = "scriptworker"
    worker = task["worker"]
    artifact_map = worker["artifact-map"]
    for map_ in artifact_map:
        map_["locale"] = "multi"
        for path_config in map_["paths"].values():
            path_config["checksums_path"] = ""
    for artifact in worker["upstream-artifacts"]:
        artifact["locale"] = "multi"
    release_properties = {
        "appName": worker["release-properties"]["app-name"],
        "appVersion": worker["release-properties"]["app-version"],
        "branch": worker["release-properties"]["branch"],
        "buildid": worker["release-properties"]["build-id"],
        "hashType": worker["release-properties"].get("hash-type", "sha512"),
        "platform": worker["release-properties"]["platform"],
    }
    scope_prefix = "project:mozillavpn:releng:beetmover:"
    task_def["scopes"] = [
        "{prefix}bucket:{bucket_scope}".format(
            prefix=scope_prefix, bucket_scope=worker["bucket"]
        ),
        "{prefix}action:{action_scope}".format(
            prefix=scope_prefix, action_scope=worker["action"]
        ),
    ]
    if worker["action"] == "push-to-releases":
        task_def["payload"] = {
            "maxRunTime": 600,
            "product": release_properties["appName"],
            "build_number": worker["build-number"],
            "version": release_properties["appVersion"],
        }
    else:
        task_def["payload"] = {
            "maxRunTime": 600,
            "artifactMap": artifact_map,
            "releaseProperties": release_properties,
            "upstreamArtifacts": worker["upstream-artifacts"],
            "upload_date": int(datetime.now().timestamp()),
        }

@payload_builder(
    "beetmover-import-from-gcs-to-artifact-registry",
    schema={
        Required("max-run-time"): int,
        Required("gcs-sources"): [str],
        Required("product"): str,
    },
)
def build_import_from_gcs_to_artifact_registry_payload(config, task, task_def):
    task_def["tags"]["worker-implementation"] = "scriptworker"
    task_def["payload"] = {
        "product": task["worker"]["product"],
        "gcs_sources": task["worker"]["gcs-sources"],
    }
