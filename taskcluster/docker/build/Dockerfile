FROM $DOCKER_IMAGE_PARENT

RUN apt-get update && \
    apt-get install --yes golang debhelper rsync flatpak flatpak-builder && \
    apt-get clean

RUN pip3 install --find-links https://pypi.pub.build.mozilla.org/pub/ glean_parser==3.5

VOLUME /builds/worker/checkouts
VOLUME /builds/worker/.cache
