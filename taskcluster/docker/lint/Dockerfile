FROM $DOCKER_IMAGE_PARENT

RUN apt-get update && \
    apt-get install --yes software-properties-common && \
    apt-get update && \
    apt-get install --yes git clang-format-11 openjdk-17-jdk python3-lxml && \
    apt-get remove --yes software-properties-common && \
    apt-get clean

RUN wget https://github.com/pinterest/ktlint/releases/download/0.42.1/ktlint && \
    chmod a+x ktlint && \
    mv ktlint /usr/local/bin/
