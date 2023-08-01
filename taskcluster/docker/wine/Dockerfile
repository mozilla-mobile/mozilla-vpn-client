# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

FROM $DOCKER_IMAGE_PARENT

ARG MSVC_MAJOR_VERSION=16

RUN apt-get update -qq \
    # We need to install tzdata before all of the other packages. Otherwise it will show an interactive dialog that
    # we cannot navigate while building the Docker image.
    && apt-get install -y tzdata \
    && apt-get install -y python3 msitools python3-simplejson git wget winbind pip libgl1-mesa-glx ninja-build libegl1 \
                       python3-six ca-certificates \
    && apt-get clean -y \
    && rm -rf /var/lib/apt/lists/*

# Get wine from winehq
RUN dpkg --add-architecture i386  \
    # We need to install tzdata before all of the other packages. Otherwise it will show an interactive dialog that
    # we cannot navigate while building the Docker image.
    && mkdir -pm755 /etc/apt/keyrings \
    && wget -O /etc/apt/keyrings/winehq-archive.key https://dl.winehq.org/wine-builds/winehq.key \
    && wget -NP /etc/apt/sources.list.d/ https://dl.winehq.org/wine-builds/ubuntu/dists/kinetic/winehq-kinetic.sources \
    && apt update \
    && apt install -y --install-recommends winehq-stable \
    && rm -rf /var/lib/apt/lists/*

#  Get cmake from kitware
RUN wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null \
    && echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ jammy main' |  tee /etc/apt/sources.list.d/kitware.list >/dev/null \
    && apt-get update \
    && apt-get install -y cmake \
    && rm -rf /var/lib/apt/lists/*


# Initialize the wine environment. Wait until the wineserver process has
# exited before closing the session, to avoid corrupting the wine prefix.
RUN wine64 wineboot --init && \
    while pgrep wineserver > /dev/null; do sleep 1; done

RUN git clone https://github.com/mstorsjo/msvc-wine /opt/msvc

WORKDIR /opt/msvc

RUN PYTHONUNBUFFERED=1 ./vsdownload.py --accept-license --major=${MSVC_MAJOR_VERSION} --dest /opt/msvc \
    && ./install.sh /opt/msvc \
    && rm lowercase fixinclude install.sh vsdownload.py \
    && rm -rf wrappers \
    && chmod -R 777 /opt/


# Pull in the Toolchain File for this container.
# %include taskcluster/docker/wine/wine_cross.toolchain
ADD topsrcdir/taskcluster/docker/wine/wine_cross.toolchain /opt/msvc/wine_cross.toolchain
ENV CMAKE_TOOLCHAIN_FILE="/opt/msvc/wine_cross.toolchain"

VOLUME /builds/worker/.cache
VOLUME /builds/worker/checkouts
