# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

FROM $DOCKER_IMAGE_PARENT

MAINTAINER Naomi Kirby <naomi@mozilla.com>

RUN apt-get update -q && \
    apt-get -y install build-essential \
                       clang \
                       cmake \
                       curl \
                       libclang-dev \
                       libgl1-mesa-dev \
                       ninja-build \
                       patchelf

## Install Qt6/X11 build dependencies
## From: https://doc.qt.io/qt-6/linux-requirements.html
RUN apt-get -y install libfontconfig1-dev \
                       libfreetype6-dev \
                       libx11-dev \
                       libx11-xcb-dev \
                       libxext-dev \
                       libxfixes-dev \
                       libxi-dev \
                       libxrender-dev \
                       libxcb1-dev \
                       libxcb-cursor-dev \
                       libxcb-glx0-dev \
                       libxcb-keysyms1-dev \
                       libxcb-image0-dev \
                       libxcb-shm0-dev \
                       libxcb-icccm4-dev \
                       libxcb-sync-dev \
                       libxcb-xfixes0-dev \
                       libxcb-shape0-dev \
                       libxcb-randr0-dev \
                       libxcb-render-util0-dev \
                       libxcb-util-dev \
                       libxcb-xinerama0-dev \
                       libxcb-xkb-dev \
                       libxkbcommon-dev \
                       libxkbcommon-x11-dev

#----------------------------------------------------------------------------------------------------------------------
#-- Worker User -------------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------------------------

# Setup the worker user
# %include taskcluster/scripts/setup-worker.sh
ADD topsrcdir/taskcluster/scripts/setup-worker.sh /root/setup-worker.sh
RUN /root/setup-worker.sh
WORKDIR /builds/worker/

# Grant the worker user the ability to install packages without login
RUN echo "worker ALL=(ALL) SETENV:NOPASSWD:/usr/bin/apt,/usr/bin/apt-get,/usr/bin/dpkg" > /etc/sudoers.d/worker-packages

#----------------------------------------------------------------------------------------------------------------------
#-- Task Setup --------------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------------------------

# %include-run-task
# %include taskcluster/scripts/build/linux_build_dpkg.sh
ADD topsrcdir/taskcluster/scripts/build/linux_build_dpkg.sh /builds/worker/builder.sh

ENV SHELL=/bin/bash \
    HOME=/builds/worker \
    PATH="/builds/worker/.local/bin:$PATH"

VOLUME /builds/worker/checkouts
VOLUME /builds/worker/.cache

# run-task expects to run as root
USER root
