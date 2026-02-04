# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

ARG DOCKER_BASE_IMAGE=almalinux:8

FROM $DOCKER_BASE_IMAGE

MAINTAINER Naomi Kirby <naomi@mozilla.com>

#----------------------------------------------------------------------------------------------------------------------
#-- Extra Packages ----------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------------------------
RUN yum -y install git python3 rpm-build rpmdevtools yum-utils sudo

# Enable extra repositories depending on base image
RUN if [[ -f /etc/almalinux-release ]]; then yum -y install dnf-plugins-core epel-release && dnf config-manager --enable powertools; fi

# Install Qt6 Build Dependencies
RUN yum -y install \
        brotli-devel \
        dbus-devel \
        libglvnd-devel \
        libxkbcommon-devel \
        libxkbcommon-x11-devel \
        fontconfig-devel \
        freetype-devel \
        xcb-util-devel \
        xcb-util-cursor-devel \
        xcb-util-image-devel \
        xcb-util-keysyms-devel \
        xcb-util-renderutil-devel \
        xcb-util-wm-devel \
        wayland-devel

# Grant the worker user the ability to install packages without login
RUN echo "worker ALL=(ALL) SETENV:NOPASSWD:/usr/bin/yum,/usr/bin/yum-builddep" > /etc/sudoers.d/worker-packages

# Also pull in packages from BuildRequires as early as possible, excluding Qt deps.
# %include linux/mozillavpn.spec
ADD topsrcdir/linux/mozillavpn.spec /root/mozillavpn-rpm.spec
RUN grep -v 'BuildRequires: qt6-' /root/mozillavpn-rpm.spec > /root/mozillavpn-no-qt6.spec
RUN yum-builddep -y /root/mozillavpn-no-qt6.spec -D "_version 0.0.1"

#----------------------------------------------------------------------------------------------------------------------
#-- Worker User -------------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------------------------

# Setup the worker user
# %include taskcluster/scripts/setup-worker.sh
ADD topsrcdir/taskcluster/scripts/setup-worker.sh /root/setup-worker.sh
RUN /root/setup-worker.sh
WORKDIR /builds/worker/

#----------------------------------------------------------------------------------------------------------------------
#-- Task Setup --------------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------------------------

# %include-run-task
# %include taskcluster/scripts/build/linux_build_rpm.sh
ADD topsrcdir/taskcluster/scripts/build/linux_build_rpm.sh /builds/worker/builder.sh

ENV SHELL=/bin/bash \
    HOME=/builds/worker \
    PATH="/builds/worker/.local/bin:$PATH"

VOLUME /builds/worker/checkouts
VOLUME /builds/worker/.task-cache/pip

# run-task expects to run as root
USER root
