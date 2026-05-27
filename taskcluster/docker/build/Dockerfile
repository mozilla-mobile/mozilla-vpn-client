FROM $DOCKER_IMAGE_PARENT

# Add external PPA, latest version of QT is 5.12.x for Ubuntu 20.04
RUN apt-get update && \
    apt-get install --yes cmake ninja-build golang debhelper rsync && \
    apt-get clean

VOLUME /builds/worker/checkouts
VOLUME /builds/worker/.task-cache/pip
