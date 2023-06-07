# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

ARG DOCKER_IMAGE_PARENT=ubuntu:20.04
FROM --platform=linux/amd64 $DOCKER_IMAGE_PARENT 

MAINTAINER Sebastian Streich <sstreich@mozilla.com>

ARG ANDROID_ARCH=android_armv7
ARG SENTRY_CLI_VERSION="2.9.0"
ENV DEBIAN_FRONTEND=noninteractive


# %include android_sdk.txt
ADD topsrcdir/android_sdk.txt /root/android_sdk.txt
# %include scripts/android/conda_setup_qt.sh
ADD topsrcdir/scripts/android/conda_setup_qt.sh /root/scripts/android/conda_setup_qt.sh
# %include scripts/android/conda_setup_sdk.sh
ADD topsrcdir/scripts/android/conda_setup_sdk.sh /root/scripts/android/conda_setup_sdk.sh



RUN cd /root/ &&\
    bash -l -c "conda activate vpn && ./scripts/android/conda_setup_sdk.sh" &&\
    bash -l -c "conda activate vpn && ./scripts/android/conda_setup_qt.sh" &&\
    bash -l -c "conda activate vpn && ./scripts/android/conda_trim.sh"
    
# Sentry tools
RUN curl -sL https://sentry.io/get-cli/ | SENTRY_CLI_VERSION=${SENTRY_CLI_VERSION} bash


VOLUME /builds/worker/.cache
VOLUME /builds/worker/checkouts

RUN rm -rf /builds/worker/.cache/** && \ 
    rm -rf /builds/worker/checkouts/**
