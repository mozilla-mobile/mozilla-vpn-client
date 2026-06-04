# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

FROM ubuntu:24.04

VOLUME /builds/worker/checkouts
VOLUME /builds/worker/.task-cache/pip
VOLUME /builds/worker/.task-cache/cargo

# Setup the worker user
# %include taskcluster/scripts/setup-worker.sh
ADD topsrcdir/taskcluster/scripts/setup-worker.sh /root/setup-worker.sh
RUN /root/setup-worker.sh
WORKDIR /builds/worker/

#----------------------------------------------------------------------------------------------------------------------
#-- Configuration -----------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------------------------

ENV CURL='curl --location --retry 5' \
    DEBIAN_FRONTEND='noninteractive' \
    LANG='en_US.UTF-8' \
    PIP_BREAK_SYSTEM_PACKAGES=1 \
    TERM='dumb'

#----------------------------------------------------------------------------------------------------------------------
#-- System ------------------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------------------------

RUN apt-get update -qq \
    # We need to install tzdata before all of the other packages. Otherwise it will show an interactive dialog that
    # we cannot navigate while building the Docker image.
    && apt-get install -y tzdata \
    && apt-get install -y python3 \
                          python3-pip \
                          python3-yaml \
                          locales \
                          git \
                          wget \
                          zip \
                          cargo-1.85 \
                          curl \
                          rpm \
                          libglib2.0-0 \
                          ccache \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Install the taskcluster script's package dependencies
# %include taskcluster/scripts/requirements.txt
ADD topsrcdir/taskcluster/scripts/requirements.txt /root/pip-requirements.txt
RUN pip install -r /root/pip-requirements.txt

RUN locale-gen "$LANG"

# %include-run-task

ENV SHELL=/bin/bash \
    HOME=/builds/worker \
    PATH="/builds/worker/.local/bin:/usr/lib/rust-1.85/bin:$PATH"

# The build will be run as worker, which has no access
# thus we should prep the folders
RUN mkdir -p /builds/worker/.cache/go-build &&\
    chown -R worker:worker /builds/worker/.cache/ &&\
    chmod -R 777 /builds/worker/

# run-task expects to run as root
USER root
