# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

ARG DOCKER_IMAGE_PARENT=ubuntu:20.04
FROM $DOCKER_IMAGE_PARENT

MAINTAINER Sebastian Streich <sstreich@mozilla.com>

ARG QT_VERSION=6.2.4

VOLUME /builds/worker/checkouts

ENV CURL='curl --location --retry 5' \
    LANG='en_US.UTF-8' \
    TERM='dumb'

RUN apt-get -q update && apt-get install -y tzdata && apt-get install -y git ccache python3 python3-pip wget libglib2.0-0 ninja-build && \
    python3 -m pip install aqtinstall && \
    # qt6 for wasm needs the desktop linux installation
    python3 -m aqt install-qt -O /opt linux desktop ${QT_VERSION} gcc_64 && \
    python3 -m aqt install-qt -O /opt linux desktop ${QT_VERSION} wasm_32 -m qtwebsockets qt5compat && \
    cd /opt/ && \
    git clone https://github.com/emscripten-core/emsdk.git && \
    cd emsdk && \
    ./emsdk install 2.0.14 && \
    ./emsdk activate 2.0.14 && \
    wget https://github.com/Kitware/CMake/releases/download/v3.24.0-rc4/cmake-3.24.0-rc4-linux-x86_64.sh && \
    chmod +x cmake-3.24.0-rc4-linux-x86_64.sh && \
    mkdir /tmp/cmake && \
    ./cmake-3.24.0-rc4-linux-x86_64.sh --skip-license --prefix=/tmp/cmake

ENV QTPATH=/opt/${QT_VERSION}/
ENV EMSDKPATH=/opt/emsdk/

ENV PATH="/opt/${QT_VERSION}/gcc_64/bin:${PATH}:/tmp/cmake/bin"
