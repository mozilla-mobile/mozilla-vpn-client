# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

FROM debian:bookworm

ARG DEB_HOST_ARCH=amd64
ARG DEB_HOST_GNU_CPU=x86_64

MAINTAINER Naomi Kirby <naomi@mozilla.com>

#----------------------------------------------------------------------------------------------------------------------
#-- Extra Packages ----------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------------------------
ENV CURL='curl --location --retry 5' \
    DEBIAN_FRONTEND='noninteractive' \
    LANG='en_US.UTF-8' \
    LC_ALL='en_US.UTF-8' \
    TERM='dumb'

## Ensure that we generate the default locale
RUN echo "locales locales/locales_to_be_generated multiselect en_US.UTF-8 UTF-8" | debconf-set-selections

RUN apt-get update -q
RUN apt-get -y install tzdata
RUN apt-get -y install build-essential \
                       clang-16 \
                       cmake \
                       curl \
                       debhelper \
                       devscripts \
                       equivs \
                       golang \
                       libclang-16-dev \
                       libdbus-1-3 \
                       libgl1-mesa-dev \
                       locales \
                       ninja-build \
                       patchelf \
                       pkg-config \
                       sudo

# Enable multiarch and add cross-compilation toolchain
RUN if [ "$DEB_HOST_ARCH" != "amd64" ]; then \
        dpkg --add-architecture $DEB_HOST_ARCH && apt-get update; \
        apt-get -y install --no-install-recommends \
                crossbuild-essential-${DEB_HOST_ARCH} \
                gcc-${DEB_HOST_GNU_CPU}-linux-gnu \
                g++-${DEB_HOST_GNU_CPU}-linux-gnu \
                binutils-${DEB_HOST_GNU_CPU}-linux-gnu; \
    fi

# Also pull in packages from build-depends as early as possible
# %include linux/debian/control
ADD topsrcdir/linux/debian/control /root/mozillavpn-dpkg-control
RUN sed -rie '/\s+(qt6-|qml6-|libqt6|qmake)/d' /root/mozillavpn-dpkg-control && \
	mk-build-deps -i -r --tool="apt-get --no-install-recommends --yes" --host-arch $DEB_HOST_ARCH /root/mozillavpn-dpkg-control

ENV RUSTUP_HOME=/usr/local/rustup \
    CARGO_HOME=/usr/local/cargo

RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | \
    sh -s -- -y --default-toolchain $(rustc --version | awk '{print $2}') --profile minimal --no-modify-path

ENV PATH="/usr/local/cargo/bin:${PATH}"

RUN if [ "$DEB_HOST_ARCH" != "amd64" ]; then \
        rustup target add --toolchain $(rustc --version | awk '{print $2}') ${DEB_HOST_GNU_CPU}-unknown-linux-gnu; \
    fi

#----------------------------------------------------------------------------------------------------------------------
#-- Worker User -------------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------------------------

# Setup the worker user
# %include taskcluster/scripts/setup-worker.sh
ADD topsrcdir/taskcluster/scripts/setup-worker.sh /root/setup-worker.sh
RUN /root/setup-worker.sh
WORKDIR /builds/worker/

# Grant the worker user the ability to install packages without login
RUN echo "worker ALL=(ALL) SETENV:NOPASSWD:/usr/bin/apt,/usr/bin/apt-get,/usr/bin/dpkg,/usr/bin/mk-build-deps" > /etc/sudoers.d/worker-packages

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
