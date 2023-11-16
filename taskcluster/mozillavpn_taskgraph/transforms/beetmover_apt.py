# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
from itertools import islice

from taskgraph import MAX_DEPENDENCIES
from taskgraph.transforms.base import TransformSequence
from taskgraph.util.dependencies import get_primary_dependency

from urllib.parse import urlparse


transforms = TransformSequence()


# Reads a dependent_task and returns a list 
# containing the location of all uploaded files
# inside their bucket.
def get_gcs_sources(dependent_task):
    # Destinations is an array of strings pointing to archive.
    # i.e https://ftp.mozilla.org/pub/vpn/releases/2.10.0/linux/
    destinations = dependent_task.task["extra"]["release_destinations"]
    if not destinations:
        return []
    assert len(destinations) == 1 # We only should expect one
    # release_artifacts is a list of upstream artifacts 
    # i.e [{'type': 'file', 'name': 'public/build/mozillavpn.deb', 'path': '/builds/worker/artifacts/mozillavpn.deb'}]
    release_artifacts = dependent_task.attributes["release-artifacts"]
    if not release_artifacts:
        return []

    gcs_path_prefix = urlparse(destinations[0]).path
    if gcs_path_prefix[0] == "/":
        gcs_path_prefix=gcs_path_prefix[1:]
    gcs_paths = []
    for artifact in release_artifacts:
        assert  artifact["name"].startswith("public/build/")
        artifact_name = artifact["name"].replace("public/build/","")
        # Make sure when we concat paths's not have double slashes
        gcs_paths.append(gcs_path_prefix+artifact_name)
    return gcs_paths



ALLOWED_SHIPPING_PHASES = [
    "ship-client",
    "promote-client"
]

@transforms.add
def filter_shipping(config, tasks):
    shipping_phase = config.params.get("shipping_phase", "")
    for task in tasks:
        if not shipping_phase in ALLOWED_SHIPPING_PHASES:
            continue
        attributes = {
            **task["attributes"],
            "shipping-phase": shipping_phase,
        } 
        task["attributes"]=attributes   
        yield task

@transforms.add
def beetmover_apt(config, tasks):
    for task in tasks:
        dep = get_primary_dependency(config, task)
        assert dep
        gcs_sources = get_gcs_sources(dep)
        if len(gcs_sources) == 0:
            # We do have nothing to ship, skip this task
            continue
        task["worker"]["gcs-sources"]=gcs_sources
        is_relpro = (
            config.params["level"] == "3"
            and config.params["tasks_for"] in task["run-on-tasks-for"]
        )
        bucket = "release" if is_relpro else "dep"
        project_name = "mozillavpn" if is_relpro else "mozillavpn:releng"

        task["scopes"] = [
            f"project:{project_name}:beetmover:apt-repo:{bucket}",
            f"project:{project_name}:beetmover:action:import-from-gcs-to-artifact-registry"
        ]

        yield task



