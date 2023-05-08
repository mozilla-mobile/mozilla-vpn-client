#!/bin/sh

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Get the project location.
GITDIR=$(cd $(dirname $0) && git rev-parse --show-toplevel)

# List all untracked and modified files and print them to stderr
# We permit the build directories to exist as per CMake build instructions.
git -C $GITDIR ls-files -m -o -x '.tmp/' -x 'build/' -x 'build-*/' >&2

## Do the same for submodules.
for SUBMODULE in $(git submodule foreach --recursive --quiet 'echo $sm_path'); do
    git -C $GITDIR/$SUBMODULE ls-files -m -o | xargs -d '\n' --replace=filename echo "$SUBMODULE/filename" >&2
done
