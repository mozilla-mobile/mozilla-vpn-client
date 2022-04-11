# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

FROM $DOCKER_IMAGE_PARENT

MAINTAINER Sebastian Streich <sstreich@mozilla.com>

# The build will be run as worker, which has no access
# thus we should prep the folders
RUN mkdir -p /builds/worker/.cache/go-build &&\
    chown -R worker:worker /builds/worker/.cache/ &&\
    chmod -R 777 /builds/worker/ 

#NDK Version:
ARG NDK_VERSION=android-ndk-r23b
ARG NDK_FILE=${NDK_VERSION}-linux.zip
ARG QT_VERSION=6.2.1
ARG ANDROID_ARCH=android_armv7
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get -q update &&\
    apt-get -yq install --no-install-recommends gnupg curl ca-certificates libglib2.0-dev &&\
    # add zulu-jdk to the 
    apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv-keys 0xB1998361219BD9C9 &&\
    curl -O https://cdn.azul.com/zulu/bin/zulu-repo_1.0.0-3_all.deb &&\
    apt-get -y  install ./zulu-repo_1.0.0-3_all.deb &&\
    # update the package sources 
    # install Azul Zulu JDK 11
    apt-get update &&\
    apt-get -y --no-install-recommends install zulu11-jdk unzip \
               python3-pip wget git rsync ccache make patch &&\
    #Install QT
    python3 -m pip install aqtinstall 
RUN python3 -m aqt install-qt --outputdir /opt linux desktop ${QT_VERSION} gcc_64
RUN python3 -m aqt install-qt --outputdir /opt linux android ${QT_VERSION} ${ANDROID_ARCH} -m all &&\
    #Download Additional stuff that is not in a repo
    wget -q https://dl.google.com/android/repository/${NDK_FILE} &&\
    wget https://dl.google.com/android/repository/commandlinetools-linux-7583922_latest.zip &&\
    wget -q https://raw.githubusercontent.com/mozilla-mobile/mozilla-vpn-client/main/requirements.txt &&\
    pip install -r requirements.txt &&\
    unzip commandlinetools-linux-7583922_latest.zip -d /opt &&\
    unzip ${NDK_FILE} -d /opt/NDK/ &&\
    echo y | /opt/cmdline-tools/bin/sdkmanager --sdk_root=/opt/android/sdk --install "platforms;android-30" &&\
    echo y | /opt/cmdline-tools/bin/sdkmanager --sdk_root=/opt/android/sdk --install "build-tools;30.0.3" &&\
    echo y | /opt/cmdline-tools/bin/sdkmanager --sdk_root=/opt/android/sdk --install "cmake;3.10.2.4988404" &&\
        # Note: Not sure why we need emulator, need to investiage the gradle dependencies
    echo y | /opt/cmdline-tools/bin/sdkmanager --sdk_root=/opt/android/sdk --install "emulator" &&\ 
    echo y | /opt/cmdline-tools/bin/sdkmanager --sdk_root=/opt/android/sdk --install "patcher;v4" &&\
    echo y | /opt/cmdline-tools/bin/sdkmanager --sdk_root=/opt/android/sdk --install "platform-tools" &&\
    # Cleanup
    rm -rf /opt/cmdline-tools &&\
    rm commandlinetools-linux-7583922_latest.zip &&\
    rm ${NDK_FILE} &&\
    rm zulu-repo_1.0.0-3_all.deb &&\ 
    rm -rf /var/lib/apt/lists/* &&\
    mkdir /opt/ccache

ENV QTPATH=/opt/${QT_VERSION}/${ANDROID_ARCH}
ENV ANDROID_SDK_ROOT="/opt/android/sdk"
ENV ANDROID_NDK_ROOT="/opt/NDK/${NDK_VERSION}"
ENV ANDROID_NDK_HOME="/opt/NDK/${NDK_VERSION}"

ENV PATH="/opt/${QT_VERSION}/gcc_64/bin:${QTPATH}/bin:/opt/cmdline-tools/bin:${PATH}"
ENV XDG_CACHE_HOME ="/opt/"

VOLUME /builds/worker/checkouts
VOLUME /builds/worker/.cache

