# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Docker image for cross-compiling the Mozilla VPN DEB packages for aarch64.
# Uses Debian Bookworm for best arm64 multiarch and dpkg cross-build support.

FROM debian:bookworm

MAINTAINER Marco mciullo@mozilla.com

ENV CURL='curl --location --retry 5' \
    DEBIAN_FRONTEND='noninteractive' \
    LANG='en_US.UTF-8' \
    LC_ALL='en_US.UTF-8' \
    TERM='dumb'

RUN echo "locales locales/locales_to_be_generated multiselect en_US.UTF-8 UTF-8" | debconf-set-selections

RUN apt-get update -q
RUN apt-get -y install \
        build-essential \
        curl \
        debhelper \
        devscripts \
        equivs \
        flex \
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

# Enable arm64 multiarch and install the cross-compilation toolchain.
RUN dpkg --add-architecture arm64 && apt-get update -q
RUN apt-get -y install \
        crossbuild-essential-arm64 \
        gcc-aarch64-linux-gnu \
        g++-aarch64-linux-gnu \
        binutils-aarch64-linux-gnu \
        pkg-config \
	cmake \
	golang

# %include requirements.in
ADD topsrcdir/requirements.in /root/requirements.in
RUN pip3 install --break-system-packages \
    --find-links https://pypi.pub.build.mozilla.org/pub/ \
    -r /root/requirements.in

# Install Rust + aarch64 cross-compilation target into a fixed system-wide
# location so it works regardless of the runtime $HOME value.
ENV RUSTUP_HOME=/usr/local/rustup \
    CARGO_HOME=/usr/local/cargo
RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | \
    sh -s -- -y --default-toolchain stable --profile minimal --no-modify-path && \
    /usr/local/cargo/bin/rustup target add aarch64-unknown-linux-gnu
ENV PATH="/usr/local/cargo/bin:${PATH}"

# Install arm64 link libraries required by the VPN application.
# Qt6 is excluded here - it is provided by the qt-linux-aarch64 toolchain artifact.
RUN apt-get -y install \
        libcap-dev:arm64 \
        libdbus-1-dev:arm64 \
        libfreetype6-dev:arm64 \
        libfontconfig1-dev:arm64 \
        libgcrypt20-dev:arm64 \
        libgl-dev:arm64 \
        libgles-dev:arm64 \
        libegl-dev:arm64 \
        libpolkit-gobject-1-dev:arm64 \
        libssl-dev:arm64 \
        libx11-dev:arm64 \
        libx11-xcb-dev:arm64 \
        libxcb1-dev:arm64 \
        libxcb-cursor-dev:arm64 \
        libxcb-glx0-dev:arm64 \
        libxcb-icccm4-dev:arm64 \
        libxcb-image0-dev:arm64 \
        libxcb-keysyms1-dev:arm64 \
        libxcb-randr0-dev:arm64 \
        libxcb-render-util0-dev:arm64 \
        libxcb-render0-dev:arm64 \
        libxcb-shape0-dev:arm64 \
        libxcb-shm0-dev:arm64 \
        libxcb-sync-dev:arm64 \
        libxcb-util-dev:arm64 \
        libxcb-xfixes0-dev:arm64 \
        libxcb-xinerama0-dev:arm64 \
        libxcb-xkb-dev:arm64 \
        libxkbcommon-dev:arm64 \
        libxkbcommon-x11-dev:arm64 \
        libwayland-dev:arm64 \
        libwayland-egl1:arm64 \
        libwayland-cursor0:arm64

# Pre-install native (x86_64) build dependencies from debian/control, stripping
# Qt6 entries (provided by the cross-compiled toolchain artifact instead).
# This is a cache-warming step; the final dep install happens in the build script.
# %include linux/debian/control
ADD topsrcdir/linux/debian/control /root/mozillavpn-dpkg-control
RUN sed -rie '/\s+(qt6-|qml6-|libqt6|qmake|golang)/d' /root/mozillavpn-dpkg-control && \
    mk-build-deps -i -r --tool="apt-get --no-install-recommends --yes" /root/mozillavpn-dpkg-control

# Setup the worker user
# %include taskcluster/scripts/setup-worker.sh
ADD topsrcdir/taskcluster/scripts/setup-worker.sh /root/setup-worker.sh
RUN /root/setup-worker.sh
WORKDIR /builds/worker/

RUN echo "worker ALL=(ALL) SETENV:NOPASSWD:/usr/bin/apt,/usr/bin/apt-get,/usr/bin/dpkg" > /etc/sudoers.d/worker-packages

# Task Setup

# %include-run-task
# %include taskcluster/scripts/build/linux_build_dpkg_aarch64.sh
ADD topsrcdir/taskcluster/scripts/build/linux_build_dpkg_aarch64.sh /builds/worker/builder.sh

ENV SHELL=/bin/bash \
    HOME=/builds/worker \
    PATH="/builds/worker/.local/bin:$PATH"

VOLUME /builds/worker/checkouts
VOLUME /builds/worker/.task-cache/pip

# run-task expects to run as root
USER root
