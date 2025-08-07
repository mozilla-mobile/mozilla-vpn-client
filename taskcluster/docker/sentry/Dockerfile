# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

FROM $DOCKER_IMAGE_PARENT

# Install the taskcluster script's package dependencies
# %include taskcluster/scripts/requirements.txt
ADD topsrcdir/taskcluster/scripts/requirements.txt /root/sentry-requirements.txt
RUN pip install -r /root/sentry-requirements.txt

# Install the get-secret script
# %include taskcluster/scripts/get-secret.py
ADD topsrcdir/taskcluster/scripts/get-secret.py /usr/local/bin/get-secret
RUN chmod +x /usr/local/bin/get-secret

# Install the sentry uploader script
# %include taskcluster/scripts/build/sentry_upload.sh
ADD topsrcdir/taskcluster/scripts/build/sentry_upload.sh /builds/worker/sentry_upload.sh
RUN chmod +x /builds/worker/sentry_upload.sh
