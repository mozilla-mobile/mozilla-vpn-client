FROM $DOCKER_IMAGE_PARENT

RUN apt-get update && \
    apt-get install --yes cmake ninja-build golang debhelper rsync && \
    apt-get clean

VOLUME /builds/worker/checkouts
VOLUME /builds/worker/.task-cache/pip
