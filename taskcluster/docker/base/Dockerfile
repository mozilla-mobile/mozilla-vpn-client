# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

FROM ubuntu:22.04

# Add worker user
RUN mkdir /builds && \
    useradd -d /builds/worker -s /bin/bash -m worker && \
    chown worker:worker /builds/worker && \
    mkdir /builds/worker/artifacts && \
    chown worker:worker /builds/worker/artifacts

WORKDIR /builds/worker/

#----------------------------------------------------------------------------------------------------------------------
#-- Configuration -----------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------------------------

ENV CURL='curl --location --retry 5' \
    LANG='en_US.UTF-8' \
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
                          cargo \
                          rpm \
                          libglib2.0-0 \
                          ccache \
    && apt-get clean

RUN pip install --upgrade pip
RUN pip install taskcluster

RUN locale-gen "$LANG"

# %include-run-task

ENV SHELL=/bin/bash \
    HOME=/builds/worker \
    PATH="/builds/worker/.local/bin:$PATH"

VOLUME /builds/worker/checkouts
VOLUME /builds/worker/.cache

# The build will be run as worker, which has no access
# thus we should prep the folders
RUN mkdir -p /builds/worker/.cache/go-build &&\
    chown -R worker:worker /builds/worker/.cache/ &&\
    chmod -R 777 /builds/worker/

# run-task expects to run as root
USER root
