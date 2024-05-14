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
                          yq \
    && apt-get clean

RUN locale-gen "$LANG"

#----------------------------------------------------------------------------------------------------------------------
#-- Task Setup --------------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------------------------

# %include taskcluster/scripts/build/linux_build_flatpak.sh
ADD topsrcdir/taskcluster/scripts/build/linux_build_flatpak.sh /root/builder.sh

# The flatpak manifest to be build should be in the source volume.
VOLUME /mnt/source

# The generated flatpaks will be sent to the artifacts volume.
VOLUME /mnt/artifacts

# Running the container builds a flatpak
CMD /root/builder.sh
