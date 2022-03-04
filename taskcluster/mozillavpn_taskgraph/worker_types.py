# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from voluptuous import Any, Required, Optional
from six import text_type
from taskgraph.util.schema import taskref_or_string
from taskgraph.transforms.task import payload_builder


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
            }
        ],
    },
)
def build_scriptworker_signing_payload(config, task, task_def):
    worker = task["worker"]

    task_def["tags"]["worker-implementation"] = "scriptworker"

    task_def["payload"] = {
        "maxRunTime": worker["max-run-time"],
        "upstreamArtifacts": worker["upstream-artifacts"],
    }

    formats = set()
    for artifacts in worker["upstream-artifacts"]:
        formats.update(artifacts["formats"])

    scope_prefix = "project:mozillavpn"
    task_def["scopes"].append(
        "{}:releng:signing:cert:{}".format(scope_prefix, worker["signing-type"])
    )
    task_def["scopes"].extend(
        [
            f"{scope_prefix}:releng:signing:format:{format}"
            for format in sorted(formats)
        ]
    )

@payload_builder(
    "scriptworker-pushapk",
    schema={
        Required("upstream-artifacts"): [
            {
                Required("taskId"): taskref_or_string,
                Required("taskType"): text_type,
                Required("paths"): [text_type],
            }
        ],
        Required("certificate-alias"): text_type,
        Required("commit"): bool,
        Required("channel"): text_type,
        Required("product"): text_type,
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
        "project:mozillavpn:releng:googleplay:product:{}".format(
            worker["product"]
        )
    )
