# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

ARG DOCKER_BASE_IMAGE=fedora:36

FROM $DOCKER_BASE_IMAGE

MAINTAINER Naomi Kirby <naomi@mozilla.com>

#----------------------------------------------------------------------------------------------------------------------
#-- Extra Packages ----------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------------------------
RUN yum -y install git rpm-build rpmdevtools yum-utils sudo

# Grant the worker user the ability to install packages without login
RUN echo "worker ALL=(ALL) SETENV:NOPASSWD:/usr/bin/yum,/usr/bin/yum-builddep" > /etc/sudoers.d/worker-packages

# Also pull in packages from BuildRequires as early as possible
# %include linux/mozillavpn.spec
ADD topsrcdir/linux/mozillavpn.spec /root/mozillavpn-rpm.spec
RUN yum-builddep -y /root/mozillavpn-rpm.spec -D "_version 0.0.1"

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
