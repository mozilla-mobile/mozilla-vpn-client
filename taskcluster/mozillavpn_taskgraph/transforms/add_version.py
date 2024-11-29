# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import os.path

from taskgraph.transforms.base import TransformSequence

transforms = TransformSequence()

# Append the VPN client version to the filenames of the release-artifacts.
@transforms.add
def append_version(config, tasks):
    # Get the version number from version.txt
    cwd = os.path.dirname(os.path.realpath(__file__))
    with open(cwd, '..', '..', 'version.txt', 'r') as fp:
        version = fp.readline().strip()

    for task in tasks:
        print(f"DEBUG: append_version on {task['name']}")
        if task["name"] not in ("repackage-deb-release", "beetmover-ship-linux64-deb", "beetmover-apt-ship-linux64-deb"):
            yield task
            continue

        # Get the release artifacts and append -<version> to their filenames.
        print(f"DEBUG: task keys: {task.keys()}")
        for artifact in task["attributes"]["release-artifacts"]:  
            print(f"DEBUG: release-artifact {artifact['name']}")
            print(f"DEBUG: release-artifact {artifact['path']}")
            if "name" in artifact:
                root, ext = os.path.splitext(artifact["name"])
                artifact["name"] = root + "-" + version + ext
            if "path" in artifact:
                root, ext = os.path.splitext(artifact["path"])
                artifact["path"] = root + "-" + version + ext

        yield task
