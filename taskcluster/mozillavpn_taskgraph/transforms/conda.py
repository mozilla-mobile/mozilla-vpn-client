# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.


import os.path

from taskgraph.transforms.base import TransformSequence

transforms = TransformSequence()

ARCH_MAP = {
    "arm64":"android_arm64_v8a",
    "armv7": "armeabi-v7a",
    "x86":"android_x86",
    "x86_64":"android_x86_64"
}


# Inflates conda-android pack tasks
# and auto fill's template stuff.
@transforms.add
def inflate_task_config(config, tasks):
    for task in tasks:
        if not task["name"].startswith("conda-android"):
            yield task
            continue
        # Name is in the format conda-android-<arch>-<qt-version>
        parts = task["name"].split("-")
        version = parts[-1]
        arch = parts[-2]

        if not version and not arch:
            raise Exception(f"The Task {task['name']} has a malformed name, use: conda-android-<arch>-<qt-version>") 
        
        task["run"]["toolchain-alias"]=f"conda-android-{arch}-{version}"
        task["treeherder"]["symbol"]=f"TL(conda-{version})"
        task["treeherder"]["platform"]=f"android/{arch})"
        task["worker"]["env"]["QT_VERSION"]=version
        task["worker"]["env"]["ANDROID_ARCH"]=ARCH_MAP[arch]
        
        yield task
