#!/bin/sh
# Generate a localized desktop entry file
SRCDIR=$(cd $(dirname $0)/../.. && pwd)
XLIFFTOOL=${SRCDIR}/scripts/utils/xlifftool.py

helpFunction() {
  echo "Usage: $0 [OPTIONS]"
  echo ""
  echo "Build options:"
  echo "  -o, --output FILE      Output desktop entry to FILE (default: stdout)"
  echo "  -b, --bindir DIR       Binary directory application for app"
  echo "  -h, --help             Print this message and exit"
  echo ""
}

BINDIR="/usr/local/bin"
OUTPUT="-"

while [[ $# -gt 0 ]]; do
  key="$1"

  case $key in
  -o | --output)
    OUTPUT="$2"
    shift
    shift
    ;;
  -b | --bindir)
    BINDIR="$2"
    shift
    shift
    ;;
  -h | --help)
    helpFunction >&2
    exit 0
    ;;
  *)
    helpFunction >&2
    exit 1
    ;;
  esac
done

if [ "${OUTPUT}" != "-" ]; then
  exec 1>"${OUTPUT}"
fi

cat << EOF
[Desktop Entry]
Name=Mozilla VPN
Version=1.5
Exec=${BINDIR}/mozillavpn ui %u
Comment=A fast, secure and easy to use VPN. Built by the makers of Firefox.
Type=Application
Icon=org.mozilla.vpn
Terminal=false
StartupNotify=true
Encoding=UTF-8
Categories=Network;Security;
MimeType=x-scheme-handler/mozilla-vpn
StartupWMClass=mozillavpn

EOF

# Generate localized names and comment.
for LOCALE_DIR in $(find ${SRCDIR}/3rdparty/i18n/ -mindepth 1 -maxdepth 1 -type d); do
  LOCALE_NAME=$(basename ${LOCALE_DIR})
  LOCALE_XLIFF=${LOCALE_DIR}/mozillavpn.xliff
  if [ ! -f ${LOCALE_XLIFF} ]; then
    continue
  fi
  ${XLIFFTOOL} ${LOCALE_XLIFF} --xform - << EOF
Name[${LOCALE_NAME}]=qtTrId("vpn.product.name")
Comment[${LOCALE_NAME}]=qtTrId("vpn.product.description")
EOF
done
