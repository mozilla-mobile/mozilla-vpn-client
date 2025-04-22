# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

ARG DOCKER_IMAGE_PARENT=ubuntu:22.04
FROM $DOCKER_IMAGE_PARENT

MAINTAINER Sebastian Streich <sstreich@mozilla.com>

ARG QT_VERSION=6.6.3

VOLUME /builds/worker/checkouts
VOLUME /builds/worker/.task-cache/pip

ENV CURL='curl --location --retry 5' \
    LANG='en_US.UTF-8' \
    TERM='dumb'

RUN apt-get -q update && apt-get install -y tzdata && \
    apt-get install -y git ccache python3 python3-pip wget libglib2.0-0 cmake ninja-build && \
    python3 -m pip install aqtinstall && \
    # qt6 for wasm needs the desktop linux installation
    python3 -m aqt install-qt -O /opt linux desktop ${QT_VERSION} gcc_64 -m qtshadertools && \
    python3 -m aqt install-qt -O /opt linux desktop ${QT_VERSION} wasm_singlethread -m qtwebsockets qt5compat qtshadertools && \
    chmod +x /opt/${QT_VERSION}/wasm_singlethread/bin/qt-cmake && \
    cd /opt/ && \
    git clone https://github.com/emscripten-core/emsdk.git && \
    cd emsdk && \
    ./emsdk install 3.1.37 && \
    ./emsdk activate 3.1.37

ENV QTPATH=/opt/${QT_VERSION}/
ENV EMSDKPATH=/opt/emsdk/

ENV PATH="/opt/${QT_VERSION}/gcc_64/bin:${PATH}"
