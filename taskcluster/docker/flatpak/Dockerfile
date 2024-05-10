# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

FROM debian:bullseye

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
                          flatpak \
                          flatpak-builder \
                          gpg-agent \
                          locales \
                          software-properties-common \
    && apt-get clean

RUN locale-gen "$LANG"

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

# %include taskcluster/scripts/build/linux_build_flatpak.sh
ADD topsrcdir/taskcluster/scripts/build/linux_build_flatpak.sh /builds/worker/builder.sh

ENV SHELL=/bin/bash \
    HOME=/builds/worker \
    PATH="/builds/worker/.local/bin:$PATH"

# The flatpak manifest to be build should be in the source volume.
VOLUME /builds/worker/source

# The generated flatpaks will be sent to the artifacts volume.
VOLUME /builds/worker/artifacts

# Running the container builds a flatpak
USER worker
CMD /builds/worker/builder.sh
