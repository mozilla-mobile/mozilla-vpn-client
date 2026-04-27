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
        python3 \
        python3-click \
        python3-jinja2 \
        python3-pip \
        python3-yaml \
        sudo \
        tzdata

RUN locale-gen "$LANG" 

# Enable arm64 multiarch and install the cross-compilation toolchain.
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

# Install Rust + aarch64 cross-compilation target into a fixed system-wide
# location so it works regardless of the runtime $HOME value.
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

RUN echo "worker ALL=(ALL) SETENV:NOPASSWD:/usr/bin/apt,/usr/bin/apt-get,/usr/bin/dpkg" > /etc/sudoers.d/worker-packages

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
