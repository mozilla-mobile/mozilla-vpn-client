# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

FROM $DOCKER_IMAGE_PARENT

MAINTAINER Sebastian Streich <sstreich@mozilla.com>

VOLUME /builds/worker/checkouts

RUN mkdir -p /builds/worker/.cache/go-build &&\
    chown -R worker:worker /builds/worker/.cache/ &&\
    chmod -R 777 /builds/worker/ 

RUN apt-get -q update &&\
    apt-get -yq install --no-install-recommends gnupg curl ca-certificates &&\
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
    python3 -m pip install aqtinstall &&\
    python3 -m aqt install --outputdir /opt 5.15.2 linux android -m qtnetworkauth &&\
    #Download Additional stuff that is not in a repo
    wget -q https://dl.google.com/android/repository/android-ndk-r20b-linux-x86_64.zip &&\
    wget https://dl.google.com/android/repository/commandlinetools-linux-7583922_latest.zip &&\
    wget -q https://raw.githubusercontent.com/mozilla-mobile/mozilla-vpn-client/main/requirements.txt &&\
    pip install -r requirements.txt &&\
    unzip commandlinetools-linux-7583922_latest.zip -d /opt &&\
    unzip android-ndk-r20b-linux-x86_64.zip -d /opt/NDK/ &&\
    echo y | /opt/cmdline-tools/bin/sdkmanager --sdk_root=/opt/android/sdk --install "platforms;android-30" &&\
    echo y | /opt/cmdline-tools/bin/sdkmanager --sdk_root=/opt/android/sdk --install "build-tools;29.0.2" &&\
    echo y | /opt/cmdline-tools/bin/sdkmanager --sdk_root=/opt/android/sdk --install "cmake;3.10.2.4988404" &&\
        # Note: Not sure why we need emulator, need to investiage the gradle dependencies
    echo y | /opt/cmdline-tools/bin/sdkmanager --sdk_root=/opt/android/sdk --install "emulator" &&\ 
    echo y | /opt/cmdline-tools/bin/sdkmanager --sdk_root=/opt/android/sdk --install "patcher;v4" &&\
    echo y | /opt/cmdline-tools/bin/sdkmanager --sdk_root=/opt/android/sdk --install "platform-tools" &&\
    # Cleanup
    rm -rf /opt/cmdline-tools &&\
    rm commandlinetools-linux-7583922_latest.zip &&\
    rm android-ndk-r20b-linux-x86_64.zip &&\
    rm zulu-repo_1.0.0-3_all.deb &&\ 
    rm -rf /var/lib/apt/lists/* &&\
    mkdir /opt/ccache


ENV QTPATH=/opt/5.15.2/
ENV ANDROID_SDK_ROOT="/opt/android/sdk"
ENV ANDROID_NDK_ROOT=/opt/NDK/android-ndk-r20b
ENV PATH="/opt/5.15.2/android/bin:/opt/cmdline-tools/bin:${PATH}"
