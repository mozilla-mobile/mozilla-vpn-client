# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

FROM $DOCKER_IMAGE_PARENT

MAINTAINER Naomi Kirby <naomi@mozilla.com>

RUN apt-get update && \
    apt-get -y install build-essential \
                       cmake \
                       ninja

VOLUME /builds/worker/checkouts
VOLUME /builds/worker/.cache
