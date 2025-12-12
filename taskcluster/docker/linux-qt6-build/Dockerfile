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
    LC_ALL='en_US.UTF-8' \
    TERM='dumb'

## Ensure that we generate the default locale
RUN echo "locales locales/locales_to_be_generated multiselect en_US.UTF-8 UTF-8" | debconf-set-selections

RUN apt-get update -q
RUN apt-get -y install tzdata
RUN apt-get -y install build-essential \
                       clang-16 \
                       curl \
                       debhelper \
                       devscripts \
                       equivs \
                       libclang-16-dev \
                       libgl1-mesa-dev \
                       locales \
                       ninja-build \
                       patchelf \
                       sudo

## Install Qt6/X11 build dependencies
## From: https://doc.qt.io/qt-6/linux-requirements.html
RUN apt-get -y install libatspi2.0-dev \
                       libdbus-1-dev \
                       libfontconfig1-dev \
                       libfreetype6-dev \
                       libssl-dev \
                       libx11-dev \
                       libx11-xcb-dev \
                       libxext-dev \
                       libxfixes-dev \
                       libxi-dev \
                       libxrender-dev \
                       libxcb1-dev \
                       libxcb-cursor-dev \
                       libxcb-glx0-dev \
                       libxcb-keysyms1-dev \
                       libxcb-image0-dev \
                       libxcb-shm0-dev \
                       libxcb-icccm4-dev \
                       libxcb-sync-dev \
                       libxcb-xfixes0-dev \
                       libxcb-shape0-dev \
                       libxcb-randr0-dev \
                       libxcb-render-util0-dev \
                       libxcb-util-dev \
                       libxcb-xinerama0-dev \
                       libxcb-xkb-dev \
                       libxkbcommon-dev \
                       libxkbcommon-x11-dev

## Install Rust
RUN mkdir -p /tmp/rust
RUN curl -sSL https://static.rust-lang.org/dist/rust-1.82.0-x86_64-unknown-linux-gnu.tar.gz -o /tmp/rust/rust-static.tar.gz
RUN echo "0265c08ae997c4de965048a244605fb1f24a600bbe35047b811c638b8fcf676b /tmp/rust/rust-static.tar.gz" | sha256sum -c -
RUN tar -C /tmp/rust -xf /tmp/rust/rust-static.tar.gz
RUN $(find /tmp/rust -name 'install.sh')
RUN rm -rf /tmp/rust

## Install Golang
RUN mkdir -p /tmp/golang
RUN curl -sSL https://go.dev/dl/go1.22.12.linux-amd64.tar.gz -o /tmp/golang/golang-static.tar.gz
RUN echo "4fa4f869b0f7fc6bb1eb2660e74657fbf04cdd290b5aef905585c86051b34d43 /tmp/golang/golang-static.tar.gz" | sha256sum -c -
RUN tar -C /usr/local -xzf /tmp/golang/golang-static.tar.gz
RUN rm -rf /tmp/golang

## Install CMake
RUN mkdir -p /tmp/cmake
RUN curl -sSL https://github.com/Kitware/CMake/releases/download/v3.26.5/cmake-3.26.5-linux-x86_64.tar.gz -o /tmp/cmake/cmake-static.tar.gz
RUN echo "130941ae3ffe4a9ee3395514787115a273a8d1ce15cb971494bb45f7e58bb3c3 /tmp/cmake/cmake-static.tar.gz" | sha256sum -c -
RUN tar -C /usr/local -xzf /tmp/cmake/cmake-static.tar.gz --strip-components=1
RUN rm -rf /tmp/cmake

#----------------------------------------------------------------------------------------------------------------------
#-- Worker User -------------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------------------------

# Setup the worker user
# %include taskcluster/scripts/setup-worker.sh
ADD topsrcdir/taskcluster/scripts/setup-worker.sh /root/setup-worker.sh
RUN /root/setup-worker.sh
WORKDIR /builds/worker/

# Grant the worker user the ability to install packages without login
RUN echo "worker ALL=(ALL) SETENV:NOPASSWD:/usr/bin/apt,/usr/bin/apt-get,/usr/bin/dpkg" > /etc/sudoers.d/worker-packages

#----------------------------------------------------------------------------------------------------------------------
#-- Task Setup --------------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------------------------

# %include-run-task
# %include taskcluster/scripts/build/linux_build_dpkg.sh
ADD topsrcdir/taskcluster/scripts/build/linux_build_dpkg.sh /builds/worker/builder.sh

ENV SHELL=/bin/bash \
    HOME=/builds/worker \
    PATH="/builds/worker/.local/bin:/usr/local/go/bin:/usr/lib/llvm-16/bin:$PATH"

VOLUME /builds/worker/checkouts
VOLUME /builds/worker/.task-cache/pip

# run-task expects to run as root
USER root
