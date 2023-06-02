# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.


import os.path

from taskgraph.transforms.base import TransformSequence

transforms = TransformSequence()


@transforms.add
def resolve_upstream_mac_task(config, tasks):
    for task in tasks:
        if "mac-notarization" not in task["dependencies"]:
            yield task
            continue

        # Remove upstream signing if notarization is available
        if "mac-notarization-macos/opt" in config.kind_dependencies_tasks:
            del task["dependencies"]["signing"]
        # Otherwise, remove notarization
        else:
            del task["dependencies"]["mac-notarization"]
        yield task
