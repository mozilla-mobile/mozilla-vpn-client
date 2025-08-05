# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set -e

# Only on a release build we have access to those secrects.
if [ "${MOZ_SCM_LEVEL}" -ge "3" ]; then
    get-secret -s project/mozillavpn/level-1/sentry -k sentry_debug_file_upload_key -f sentry_debug_file_upload_key
    sentry-cli login --auth-token $(cat sentry_debug_file_upload_key)   
else
    echo "Sentry upload requires task level 3" >&2
fi

# Auth using the sentry upload key
SENTRY_UPLOAD_ARGS="--org mozilla -p vpn-client"

# Upload support for macOS dSYM bundles.
upload_dsym_bundle() {
    # Upload symbol files
    find $1/Contents/Resources/DWARF -type f | while read DBGFILE; do
        sentry-cli debug-files upload ${SENTRY_UPLOAD_ARGS} "${DBGFILE}"
    done
    # Upload source bundles
    find $1/Contents/Resources/Sources -type f -name '*.src.zip' | while read SRCFILE; do
        sentry-cli debug-files upload ${SENTRY_UPLOAD_ARGS} --type sourcebundle "${SRCFILE}"
    done
}

# Upload the fetched artifacts to sentry
find -mindepth 1 -maxdepth 1 ${MOZ_FETCHES_DIR} | while read FILENAME; do
    case "${FILENAME}" in
        *.dSYM)
            upload_dsym_bundle ${FILENAME}
            ;;

        *.pdb)
            sentry-cli debug-files upload ${SENTRY_UPLOAD_ARGS} ${FILENAME}
            ;;

        *.src.zip)
            sentry-cli debug-files upload ${SENTRY_UPLOAD_ARGS} --type sourcebundle ${FILENAME}
            ;;

        *)
            echo "Ignoring unsupported file: ${FILENAME}"
            ;;
    esac
done
