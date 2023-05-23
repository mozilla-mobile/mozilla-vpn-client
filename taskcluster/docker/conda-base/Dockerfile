# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

FROM $DOCKER_IMAGE_PARENT

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
# %include requirements.txt
ADD topsrcdir/requirements.txt /root/requirements.txt
# %include taskcluster/requirements.txt
ADD topsrcdir/taskcluster/requirements.txt /root/taskcluster/requirements.txt
# %include taskcluster/scripts/requirements.txt
ADD topsrcdir/taskcluster/scripts/requirements.txt /root/taskcluster/scripts/requirements.txt

ENV PATH="/opt/conda/bin/:$PATH"
ENV CONDA_HOME=/opt/conda/
RUN su worker && \
    /opt/conda/bin/conda env create -f /root/env.yml -n vpn && \ 
    conda init bash