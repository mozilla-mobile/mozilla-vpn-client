FROM $DOCKER_IMAGE_PARENT

# Add external PPA, latest version of QT is 5.12.x for Ubuntu 20.04
RUN apt-get update && \
    apt-get install --yes golang debhelper rsync && \
    apt-get clean

RUN pip3 install --find-links https://pypi.pub.build.mozilla.org/pub/ glean_parser==13.0.0

VOLUME /builds/worker/checkouts
VOLUME /builds/worker/.cache
