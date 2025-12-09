# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

ARG DOCKER_BASE_IMAGE=ubuntu:jammy

FROM $DOCKER_BASE_IMAGE

MAINTAINER Naomi Kirby <naomi@mozilla.com>

#----------------------------------------------------------------------------------------------------------------------
#-- Extra Packages ----------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------------------------
ENV CURL='curl --location --retry 5' \
    DEBIAN_FRONTEND='noninteractive' \
    LANG='en_US.UTF-8' \
    TERM='dumb'

RUN apt-get update \
    && apt-get -y install --no-install-recommends \
      build-essential \
      ca-certificates \
      curl \
      debhelper \
      devscripts \
      equivs \
      fakeroot \
      git \
      locales \
      sudo

RUN locale-gen "$LANG"

# Install rustup first to meet Glean's minimum requirement (1.82)
# Ubuntu Jammy and Noble have older cargo versions, so we use rustup to get a newer one
RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y --default-toolchain 1.82.0
ENV PATH="/root/.cargo/bin:${PATH}"

# Also pull in packages from build-depends as early as possible
# %include linux/debian/control
ADD topsrcdir/linux/debian/control /root/mozillavpn-dpkg-control
RUN mk-build-deps -i -r  --tool="apt-get --no-install-recommends --yes" \
        /root/mozillavpn-dpkg-control

# Cleanup after package installation
RUN apt-get clean && rm -rf /var/lib/apt/lists/*

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
