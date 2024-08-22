#!/bin/sh
#
# Update the manifest for rust crates used by the VPN project.
#
SRCDIR=$(dirname $0)

# Create a temporary directory for the checkouts
TMPDIR=$(mktemp -d)
function cleanup(){ rm -rf $TMPDIR; }
trap cleanup EXIT

# Grab the latest flatpak builder tools
curl -sSL https://github.com/flatpak/flatpak-builder-tools/raw/master/cargo/flatpak-cargo-generator.py -o $TMPDIR/flatpak-cargo-generator.py

if [ $# -ge 2 ]; then
  # The caller can provide a Cargo.lock file as an argument
  CARGO_LOCK_FILE=$1
else
  # Otherwise - we need to download the Cargo.lock from the source
  CARGO_LOCK_FILE=$TMPDIR/Cargo.lock
  GITINFO=$(python -m yq -o=json '.modules[-1].sources[0]' ${SRCDIR}/org.mozilla.vpn.yml)
  GITVERSION=$(echo "${GITINFO}" | jq -r '.commit,.tag,.branch | select(. != null)' | head -1)
  # TODO: Looking at the source's .url would be better
  curl -sSL https://github.com/mozilla-mobile/mozilla-vpn-client/raw/${GITVERSION}/Cargo.lock -o $CARGO_LOCK_FILE
fi

# Generate the cargo dependencies and convert to YAML
python $TMPDIR/flatpak-cargo-generator.py -o $TMPDIR/generated-sources.json $CARGO_LOCK_FILE
python -m yq -y -w120 $TMPDIR/generated-sources.json > ${SRCDIR}/flatpak-vpn-crates.yaml
