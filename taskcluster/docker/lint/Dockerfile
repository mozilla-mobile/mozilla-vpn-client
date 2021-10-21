FROM $DOCKER_IMAGE_PARENT

# Add external PPA, latest version of QT is 5.12.x for Ubuntu 20.04
RUN apt-get update && \
    apt-get install --yes software-properties-common && \
    apt-get update && \
    add-apt-repository ppa:beineri/opt-qt-5.15.2-focal -y && \
    apt-get install --yes git qt515tools clang-format-11 openjdk-11-jdk python3-lxml && \
    apt-get remove --yes software-properties-common && \
    apt-get clean

RUN wget https://github.com/pinterest/ktlint/releases/download/0.42.1/ktlint && \
    chmod a+x ktlint && \
    mv ktlint /usr/local/bin/
