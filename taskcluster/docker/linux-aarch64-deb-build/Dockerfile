# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Docker image for cross-compiling the Mozilla VPN DEB packages for aarch64.
# Uses Debian Bookworm for best arm64 multiarch and dpkg cross-build support.

ARG DOCKER_BASE_IMAGE=debian:bookworm

FROM $DOCKER_BASE_IMAGE

MAINTAINER Marco mciullo@mozilla.com

ENV CURL='curl --location --retry 5' \
    DEBIAN_FRONTEND='noninteractive' \
    LANG='en_US.UTF-8' \
    LC_ALL='en_US.UTF-8' \
    TERM='dumb'

## Ensure that we generate the default locale
RUN echo "locales locales/locales_to_be_generated multiselect en_US.UTF-8 UTF-8" | debconf-set-selections

RUN apt-get update -q \
    && apt-get -y install --no-install-recommends\
        build-essential \
        curl \
        debhelper \
        devscripts \
        equivs \
        flex \
	git \
        locales \
        ninja-build \
        patchelf \
        sudo \
        tzdata

# Enable arm64 multiarch and install the cross-compilation toolchain
RUN dpkg --add-architecture arm64 && apt-get update -q
RUN apt-get -y install --no-install-recommends \
        crossbuild-essential-arm64 \
        gcc-aarch64-linux-gnu \
        g++-aarch64-linux-gnu \
        binutils-aarch64-linux-gnu \
        pkg-config \
	cmake \
	golang

# Native (x86_64) runtime libraries required by Qt host tools
RUN apt-get -y install --no-install-recommends \
        libxkbcommon0 \
        libdbus-1-3 \
        libglib2.0-0 \
        libzstd1 \
        libgl1 \
        libegl1 \
        libfreetype6 \
        libpng16-16 \
        libharfbuzz0b \
        libpcre2-8-0

# Install Qt build dependencies and MozillaVPN build dependencies needed when building
# with statically linked Qt

RUN apt install -y libdbus-1-dev:arm64 \
      libegl-dev:arm64 \
      libfontconfig1-dev:arm64 \
      libfreetype6-dev:arm64 \
      libgles-dev:arm64 \
      libssl-dev:arm64 \
      libwayland-dev:arm64 \
      libx11-dev:arm64 \
      libx11-xcb-dev:arm64 \
      libxcb1-dev:arm64 \
      libxcb-cursor-dev:arm64 \
      libxcb-glx0-dev:arm64 \
      libxcb-icccm4-dev:arm64 \
      libxcb-image0-dev:arm64 \
      libxcb-keysyms1-dev:arm64 \
      libxcb-randr0-dev:arm64 \
      libxcb-render0-dev:arm64 \
      libxcb-render-util0-dev:arm64 \
      libxcb-shape0-dev:arm64 \
      libxcb-shm0-dev:arm64 \
      libxcb-sync-dev:arm64 \
      libxcb-util-dev:arm64 \
      libxcb-xfixes0-dev:arm64 \
      libxcb-xinerama0-dev:arm64 \
      libxcb-xkb-dev:arm64 \
      libxkbcommon-x11-dev:arm64

# Install Rust + aarch64 cross-compilation target
ENV RUSTUP_HOME=/usr/local/rustup \
    CARGO_HOME=/usr/local/cargo
RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | \
    sh -s -- -y --default-toolchain stable --profile minimal --no-modify-path && \
    /usr/local/cargo/bin/rustup target add aarch64-unknown-linux-gnu
ENV PATH="/usr/local/cargo/bin:${PATH}"

# Setup the worker user
# %include taskcluster/scripts/setup-worker.sh
ADD topsrcdir/taskcluster/scripts/setup-worker.sh /root/setup-worker.sh
RUN /root/setup-worker.sh
WORKDIR /builds/worker/

RUN echo "worker ALL=(ALL) SETENV:NOPASSWD:/usr/bin/apt,/usr/bin/apt-get,/usr/bin/dpkg,/usr/bin/mk-build-deps" > /etc/sudoers.d/worker-packages

# Task Setup

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
