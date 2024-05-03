# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

ARG DOCKER_BASE_IMAGE=ubuntu:20:04

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
    && apt-get install -y tzdata \
    && apt-get install -y debhelper \
                          devscripts \
                          equivs \
                          gpg-agent \
                          locales \
                          software-properties-common \
                          sudo \
    && apt-get clean

ARG QTPPA
RUN if [ -n "${QTPPA}" ]; then add-apt-repository -y ${QTPPA}; fi

RUN locale-gen "$LANG"

# Also pull in packages from build-depends as early as possible
# %include linux/debian/control
ADD topsrcdir/linux/debian/control /root/mozillavpn-dpkg-control
RUN mk-build-deps -i -r  --tool="apt-get --no-install-recommends --yes" \
        /root/mozillavpn-dpkg-control

#----------------------------------------------------------------------------------------------------------------------
#-- Worker User -------------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------------------------

# Setup the worker user
# %include taskcluster/scripts/setup-worker.sh
ADD topsrcdir/taskcluster/scripts/setup-worker.sh /root/setup-worker.sh
RUN /root/setup-worker.sh
WORKDIR /builds/worker/

# Grant the worker user the ability to install packages without login
RUN echo "worker ALL=(ALL) SETENV:NOPASSWD:/usr/bin/apt,/usr/bin/apt-get,/usr/bin/dpkg,/usr/bin/add-apt-repository" > /etc/sudoers.d/worker-packages

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
