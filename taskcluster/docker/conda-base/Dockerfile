# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

ARG DOCKER_BASE_IMAGE=ubuntu:20:04

FROM $DOCKER_BASE_IMAGE

# Add worker user
RUN mkdir /builds && \
    useradd -d /builds/worker -s /bin/bash -m worker && \
    chown worker:worker /builds/worker && \
    mkdir /builds/worker/artifacts && \
    chown worker:worker /builds/worker/artifacts

WORKDIR /builds/worker/

#----------------------------------------------------------------------------------------------------------------------
#-- Configuration -----------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------------------------

ENV CURL='curl --location --retry 5' \
    LANG='en_US.UTF-8' \
    TERM='dumb'

#----------------------------------------------------------------------------------------------------------------------
#-- System ------------------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------------------------

RUN apt-get update -qq \
    # We need to install tzdata before all of the other packages. Otherwise it will show an interactive dialog that
    # we cannot navigate while building the Docker image.
    && apt-get install -y tzdata \
    && apt-get install -y locales bzip2 ca-certificates \
                          git \
                          wget \
                          zip \
                          cargo \
                          rpm \
    && apt-get clean

RUN pip install --upgrade pip
RUN pip install taskcluster

RUN locale-gen "$LANG"

# %include-run-task

ENV SHELL=/bin/bash \
    HOME=/builds/worker \
    PATH="/builds/worker/.local/bin:$PATH"

VOLUME /builds/worker/checkouts
VOLUME /builds/worker/.cache

# The build will be run as worker, which has no access
# thus we should prep the folders
RUN mkdir -p /builds/worker/.cache/go-build &&\
    chown -R worker:worker /builds/worker/.cache/ &&\
    chmod -R 777 /builds/worker/

# run-task expects to run as root
USER root


ARG CONDA_VERSION=py310_23.3.1-0 
ARG CONDA_SHA_SUM="aef279d6baea7f67940f16aad17ebe5f6aac97487c7c03466ff01f4819e5a651"
ENV MINICONDA_URL="https://repo.anaconda.com/miniconda/Miniconda3-${CONDA_VERSION}-Linux-x86_64.sh"  

# Setup Conda Base Enviroment
RUN wget "${MINICONDA_URL}" -O miniconda.sh -q &&  \   
    echo "${CONDA_SHA_SUM} miniconda.sh" > shasum && \
    sha256sum --check --status shasum && \
    mkdir -p /opt && \
    bash miniconda.sh -b -p /opt/conda && \     
    rm miniconda.sh shasum &&  \
    ln -s /opt/conda/etc/profile.d/conda.sh /etc/profile.d/conda.sh && \ 
    echo ". /opt/conda/etc/profile.d/conda.sh" >> ~/.bashrc && \    
    echo "conda activate base" >> ~/.bashrc && \
    find /opt/conda/ -follow -type f -name '*.a' -delete && \
    find /opt/conda/ -follow -type f -name '*.js.map' -delete &&  \
    /opt/conda/bin/conda clean -afy

# %include env.yml
ADD topsrcdir/env.yml /root/env.yml

RUN conda create env -f /root/env.yml -n vpn