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
# %include taskcluster/scripts/toolchain/bundle_flatpak_sdk.sh
ADD topsrcdir/taskcluster/scripts/toolchain/bundle_flatpak_sdk.sh /root/bundle_sdk.sh

# Running the container builds a flatpak
WORKDIR /build/workdir
CMD /root/builder.sh
