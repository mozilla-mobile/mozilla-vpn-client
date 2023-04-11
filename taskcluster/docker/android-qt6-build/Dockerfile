# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

ARG DOCKER_IMAGE_PARENT=ubuntu:20.04
FROM $DOCKER_IMAGE_PARENT

MAINTAINER Sebastian Streich <sstreich@mozilla.com>

#NDK Version:
ARG NDK_VERSION=android-ndk-r23b
ARG NDK_FILE=${NDK_VERSION}-linux.zip
ARG QT_VERSION=6.2.1
ARG CMAKE_VERSION=3.22.1
ARG ANDROID_ARCH=android_armv7
ARG SENTRY_CLI_VERSION="2.9.0"
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get -q update &&\
    apt-get -yq install --no-install-recommends gnupg curl ca-certificates libglib2.0-dev cargo &&\
    # add zulu-jdk to the
    apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv-keys 0xB1998361219BD9C9 &&\
    curl -O https://cdn.azul.com/zulu/bin/zulu-repo_1.0.0-3_all.deb &&\
    apt-get -y  install ./zulu-repo_1.0.0-3_all.deb &&\
    rm zulu-repo_1.0.0-3_all.deb &&\
    # update the package sources
    # install Azul Zulu JDK 11
    apt-get update &&\
    apt-get -y --no-install-recommends install zulu11-jdk unzip \
               python3-pip wget git rsync ccache make patch &&\
    #QT Installer
    python3 -m pip install aqtinstall
# Rust
RUN wget https://sh.rustup.rs && CARGO_HOME=/opt/.cargo sh index.html -y &&\
    /opt/.cargo/bin/rustup target add x86_64-linux-android i686-linux-android armv7-linux-androideabi aarch64-linux-android
# Sentry tools
RUN curl -sL https://sentry.io/get-cli/ | SENTRY_CLI_VERSION=${SENTRY_CLI_VERSION} bash
# Python deps
RUN wget -q https://raw.githubusercontent.com/mozilla-mobile/mozilla-vpn-client/main/requirements.txt &&\
    pip install -r requirements.txt 
# QT Target
RUN python3 -m aqt install-qt --outputdir /opt linux desktop ${QT_VERSION} gcc_64
# QT Host Tools
RUN python3 -m aqt install-qt --outputdir /opt linux android ${QT_VERSION} ${ANDROID_ARCH} -m all 
# Install NDK
RUN wget -q https://dl.google.com/android/repository/${NDK_FILE} &&\
    unzip ${NDK_FILE} -d /opt/NDK/ &&\
    rm ${NDK_FILE}
# Install Android SDK Tools
RUN wget -q https://dl.google.com/android/repository/commandlinetools-linux-7583922_latest.zip &&\
    unzip commandlinetools-linux-7583922_latest.zip -d /opt &&\
    rm commandlinetools-linux-7583922_latest.zip
RUN echo y | /opt/cmdline-tools/bin/sdkmanager --sdk_root=/opt/android/sdk --install "platforms;android-31" &&\
    echo y | /opt/cmdline-tools/bin/sdkmanager --sdk_root=/opt/android/sdk --install "platforms;android-33" &&\
    # :qtBindings:compileDebugAidl requires build-tools;29.0.2
    echo y | /opt/cmdline-tools/bin/sdkmanager --sdk_root=/opt/android/sdk --install "build-tools;29.0.2" &&\
    echo y | /opt/cmdline-tools/bin/sdkmanager --sdk_root=/opt/android/sdk --install "build-tools;30.0.3" &&\
    echo y | /opt/cmdline-tools/bin/sdkmanager --sdk_root=/opt/android/sdk --install "build-tools;31.0.0" &&\
    echo y | /opt/cmdline-tools/bin/sdkmanager --sdk_root=/opt/android/sdk --install "build-tools;33.0.0" &&\
    # Todo: Dedupe cmake deps
    echo y | /opt/cmdline-tools/bin/sdkmanager --sdk_root=/opt/android/sdk --install "cmake;3.10.2.4988404" &&\
    echo y | /opt/cmdline-tools/bin/sdkmanager --sdk_root=/opt/android/sdk --install "cmake;${CMAKE_VERSION}" &&\
        # Note: Not sure why we need emulator, need to investiage the gradle dependencies
    echo y | /opt/cmdline-tools/bin/sdkmanager --sdk_root=/opt/android/sdk --install "emulator" &&\
    echo y | /opt/cmdline-tools/bin/sdkmanager --sdk_root=/opt/android/sdk --install "patcher;v4" &&\
    echo y | /opt/cmdline-tools/bin/sdkmanager --sdk_root=/opt/android/sdk --install "platform-tools" &&\
    # Cleanup
    mkdir /opt/ccache

ENV QTPATH=/opt/${QT_VERSION}/${ANDROID_ARCH}
ENV ANDROID_SDK_ROOT="/opt/android/sdk"
ENV ANDROID_NDK_ROOT="/opt/NDK/${NDK_VERSION}"
ENV ANDROID_NDK_HOME="/opt/NDK/${NDK_VERSION}"
ENV QT_HOST_PATH=/opt/${QT_VERSION}/gcc_64

ENV PATH="/opt/${QT_VERSION}/gcc_64/bin:${QTPATH}/bin:/opt/cmdline-tools/bin:/opt/android/sdk/cmake/${CMAKE_VERSION}/bin:${ANDROID_NDK_ROOT}/toolchains/llvm/prebuilt/linux-x86_64/bin:/opt/.cargo/bin:${PATH}"
ENV XDG_CACHE_HOME ="/opt/"

VOLUME /builds/worker/checkouts
VOLUME /builds/worker/.cache
