# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

FROM $DOCKER_IMAGE_PARENT

MAINTAINER Sebastian Streich <sstreich@mozilla.com>

ARG QT_VERSION=6.2.4

VOLUME /builds/worker/checkouts

RUN mkdir -p /builds/worker/.cache/go-build &&\
    chown -R worker:worker /builds/worker/.cache/ &&\
    chmod -R 777 /builds/worker/ 

RUN apt-get -q update && apt-get install -y git ccache python3 libglib2.0-0 &&\
     python3 -m pip install aqtinstall &&\
     # qt6 for wasm needs the desktop linux installation
     python3 -m aqt install-qt -O /opt linux desktop ${QT_VERSION} gcc_64 &&\
     python3 -m aqt install-qt -O /opt linux desktop ${QT_VERSION} wasm_32 -m qtwebsockets qt5compat &&\
     # see: https://wiki.qt.io/Qt_6.2_Known_Issues#WebAssembly
     sed '/sse/,+5 d' /opt/${QT_VERSION}/wasm_32/mkspecs/features/wasm/wasm.prf > /tmp/wasm.prf &&\
     mv /tmp/wasm.prf /opt/${QT_VERSION}/wasm_32/mkspecs/features/wasm/wasm.prf &&\
     cd /opt/ &&\
     git clone https://github.com/emscripten-core/emsdk.git &&\
     cd emsdk &&\
     ./emsdk install 2.0.14 &&\
     ./emsdk activate 2.0.14 

ENV QTPATH=/opt/${QT_VERSION}/
ENV EMSDKPATH=/opt/emsdk/

ENV PATH="/opt/${QT_VERSION}/gcc_64/bin:${PATH}"
