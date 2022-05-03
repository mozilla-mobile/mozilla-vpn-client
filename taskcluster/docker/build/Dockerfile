FROM $DOCKER_IMAGE_PARENT

# Add external PPA, latest version of QT is 5.12.x for Ubuntu 20.04
RUN apt-get update && \
    apt-get install --yes golang debhelper rsync && \
    apt-get clean

RUN pip3 install --find-links https://pypi.pub.build.mozilla.org/pub/ glean_parser==3.5


RUN  mkdir -p build/archive && \
     # Install grcov into the build directory, to reduce downstream compilation times.
     cargo install --root $(pwd)/build grcov  &&\
     # Add external PPA for Qt6 support on Ubuntu 20.04
    sudo add-apt-repository ppa:okirby/qt6-backports &&\

          # Download the build and runtime package dependencies.
    sudo apt-get -o "Dir::Cache::archives=$(pwd)/build/archive" install -y \
               $(./scripts/linux/getdeps.py -a linux/debian/control.qt6) &&\
    sudo chown -R $USER:$USER build/archive


VOLUME /builds/worker/checkouts
VOLUME /builds/worker/.cache
