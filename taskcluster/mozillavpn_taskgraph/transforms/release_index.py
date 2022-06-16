# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.


from taskgraph.transforms.base import TransformSequence
transforms = TransformSequence()


@transforms.add
def add_release_index(config, tasks):
   for task in tasks:
        if "add-release-index" in task:
            release_index = task.pop("add-release-index")
            if release_index and int(config.params["level"]) == 3:
                name = task['name'].split("/")[0]
                version = config.params["app_version"]
                route = f"index.mozillavpn.v2.mozillavpn.releases.{version}.{name}"
                task.setdefault("routes", []).append(route)
        yield task
