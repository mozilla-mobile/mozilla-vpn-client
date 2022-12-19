# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

ARG DOCKER_BASE_IMAGE=fedora:36

FROM $DOCKER_BASE_IMAGE

MAINTAINER Owen Kirby <okirby@mozilla.com>

#----------------------------------------------------------------------------------------------------------------------
#-- Extra Packages ----------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------------------------
# TODO: What are we missing?
RUN yum -y install rpm-build rpmdevtools yum-utils sudo

# Grant the worker user the ability to install packages without login
RUN echo "worker ALL=(ALL) SETENV:NOPASSWD:/usr/bin/yum,/usr/bin/yum-builddep" > /etc/sudoers.d/worker-packages

#----------------------------------------------------------------------------------------------------------------------
#-- Worker User -------------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------------------------

# Add worker user
RUN mkdir /builds && \
    useradd -d /builds/worker -s /bin/bash -m worker && \
    chown worker:worker /builds/worker && \
    mkdir /builds/worker/artifacts && \
    chown worker:worker /builds/worker/artifacts

WORKDIR /builds/worker/

#----------------------------------------------------------------------------------------------------------------------
#-- Task Setup --------------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------------------------

# %include-run-task
# %include taskcluster/scripts/build/linux.sh
ADD topsrcdir/taskcluster/scripts/build/linux_build_rpm.sh /builds/worker/builder.sh

VOLUME /builds/worker/checkouts
VOLUME /builds/worker/.cache

# run-task expects to run as root
USER root
