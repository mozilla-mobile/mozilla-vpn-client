# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

FROM debian:bookworm

MAINTAINER Naomi Kirby <naomi@mozilla.com>

#----------------------------------------------------------------------------------------------------------------------
#-- Extra Packages ----------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------------------------
ENV CURL='curl --location --retry 5' \
    DEBIAN_FRONTEND='noninteractive' \
    LANG='en_US.UTF-8' \
    LC_ALL='en_US.UTF-8' \
    TERM='dumb'

## Ensure that we generate the default locale
RUN echo "locales locales/locales_to_be_generated multiselect en_US.UTF-8 UTF-8" | debconf-set-selections

RUN apt-get update -q
RUN apt-get -y install tzdata
RUN apt-get -y install build-essential \
                       clang-16 \
                       curl \
                       debhelper \
                       devscripts \
                       equivs \
                       libclang-16-dev \
                       libgl1-mesa-dev \
                       locales \
                       ninja-build \
                       patchelf \
                       sudo

## Install Qt6/X11 build dependencies
## From: https://doc.qt.io/qt-6/linux-requirements.html
RUN apt-get -y install libatspi2.0-dev \
                       libdbus-1-dev \
                       libfontconfig1-dev \
                       libfreetype6-dev \
                       libssl-dev \
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

## Install wayland build dependencies
RUN apt -y install libwayland-dev \
                   libwayland-egl-backend-dev \
                   wayland-protocols

# Also pull in packages from build-depends as early as possible
# %include linux/debian/control
ADD topsrcdir/linux/debian/control /root/mozillavpn-dpkg-control
RUN sed -rie '/\s+(qt6-|qml6-|libqt6|qmake)/d' /root/mozillavpn-dpkg-control && \
	mk-build-deps -i -r  --tool="apt-get --no-install-recommends --yes" /root/mozillavpn-dpkg-control

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
VOLUME /builds/worker/.task-cache/pip

# run-task expects to run as root
USER root
