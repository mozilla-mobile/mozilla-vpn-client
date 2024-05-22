#!/usr/bin/env python3

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import json
import os
import subprocess
import sys
import tempfile
import urllib.parse
import urllib.request

def get_task_payload():
    rootUrl = os.environ.get("TASKCLUSTER_ROOT_URL", "https://firefox-ci-tc.services.mozilla.com")
    taskUrl = urllib.parse.urljoin(rootUrl, f'api/queue/v1/task/{os.environ.get("TASK_ID")}')
    with urllib.request.urlopen(taskUrl) as req:
        return json.load(req)["payload"]

# Load the podman image and return its description in JSON
def load_podman_image(path):
    # Load the image and try to grab the tag.
    tag = None
    args = ['podman', 'load', '-i', path]
    with subprocess.Popen(args, stdout=subprocess.PIPE, stderr=sys.stderr, text=True) as proc:
        for line in proc.stdout:
            print(f'{line}') # Foreward to stdout
            if line.startswith('Loaded image'):
                i = line.index(':')
                if i > 0:
                    tag = line[i+1:].strip()

    # Describe the podman image. 
    x = subprocess.run(['podman', 'image', 'inspect', tag], stdout=subprocess.PIPE, stderr=sys.stderr)
    return json.loads(x.stdout)[0]

moz_fetches_dir = os.path.realpath(os.environ.get("MOZ_FETCHES_DIR"))

# Load the podman image.
image_filename = os.path.join(moz_fetches_dir, 'image.tar.zst')
print(f"Loading podman image from: {image_filename}")
sys.stdout.flush()
image_data = load_podman_image(image_filename)
image_config = image_data["Config"]

# Prepare the task arguments
worker_env = get_task_payload()["env"]
worker_args = ['podman', 'run', '--rm', '--privileged']

# Mount volumes into the container
volumes = image_config["Volumes"]
if '/mnt/checkouts' in volumes:
    vcs_path = os.path.realpath(os.environ.get("VCS_PATH"))
    worker_args.append(f"--volume={vcs_path}:/mnt/checkouts:ro")
if '/mnt/fetches' in volumes:
    worker_args.append(f"--volume={moz_fetches_dir}:/mnt/fetches:ro")
if '/mnt/artifacts' in volumes:
    artifact_dir = os.path.join(os.environ.get("TASK_WORKDIR"), 'artifacts')
    worker_args.append(f"--volume={artifact_dir}:/mnt/artifacts:rw")
    os.makedirs(artifact_dir, exist_ok=True)

# Propagate the task environment into the container, while tweaking paths.
worker_env["VCS_PATH"] = '/mnt/checkout'
worker_env["MOZ_FETCHES_DIR"] = '/mnt/fetches'
worker_env["TASK_WORKDIR"] = image_config.get("WorkingDir", "/")
worker_env_file = tempfile.NamedTemporaryFile(mode='w+', prefix='env-', suffix='.txt', encoding='utf-8')
for key in worker_env:
    print(f'{key}={worker_env[key]}', file=worker_env_file)
worker_env_file.flush()
worker_args.append(f'--env-file={worker_env_file.name}')

# Last argument should be the image name followed by the script arguments.
worker_args.append(image_data["Id"])
worker_args.extend(sys.argv[1:])

# Run the requested task in the container
if len(image_data["RepoTags"]) > 0:
    print(f'Running podman image: {image_data["RepoTags"][0]}')
else:
    print(f'Running podman image: {image_data["Id"]}')
sys.stdout.flush()
x = subprocess.run(worker_args, stdout=sys.stdout, stderr=sys.stderr)
sys.exit(x.returncode)
