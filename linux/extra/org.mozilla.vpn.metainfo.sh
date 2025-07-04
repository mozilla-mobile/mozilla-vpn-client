#!/bin/sh
# Generate a localized metainfo file
SRCDIR=$(cd $(dirname $0)/../.. && pwd)
XLIFFTOOL=${SRCDIR}/scripts/utils/xlifftool.py

helpFunction() {
  echo "Usage: $0 [OPTIONS]"
  echo ""
  echo "Build options:"
  echo "  -o, --output FILE      Output metainfo to FILE (default: stdout)"
  echo "  -d, --datadir DIR      Data directory application for app"
  echo "  -g, --gitref REF       Github ref external URLs"
  echo "  -h, --help             Print this message and exit"
  echo ""
}

DATADIR="/usr/local/share"
GITREF="refs/heads/main"
OUTPUT="-"

while [[ $# -gt 0 ]]; do
  key="$1"

  case $key in
  -o | --output)
    OUTPUT="$2"
    shift
    shift
    ;;
  -d | --datadir)
    DATADIR="$2"
    shift
    shift
    ;;
  -g | --gitref)
    GITREF="$2"
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
<?xml version="1.0" encoding="UTF-8"?>
<component type="desktop-application">
  <id>org.mozilla.vpn</id>

  <name>Mozilla VPN</name>
  <summary>A fast, secure and easy to use VPN. Built by the makers of Firefox</summary>

EOF

# Generate localized name and summary.
for LOCALE_DIR in $(find ${SRCDIR}/3rdparty/i18n/ -mindepth 1 -maxdepth 1 -type d); do
  LOCALE_BCP47=$(basename ${LOCALE_DIR} | tr '_' '-')
  LOCALE_XLIFF=${LOCALE_DIR}/mozillavpn.xliff
  if [ ! -f ${LOCALE_XLIFF} ]; then
    continue
  fi
  ${XLIFFTOOL} ${LOCALE_XLIFF} --xform - << EOF
  <name xml:lang="${LOCALE_BCP47}">qtTrId("vpn.product.name")</name>
  <summary xml:lang="${LOCALE_BCP47}">qtTrId("vpn.product.description")</summary>
EOF
done

cat << EOF

  <metadata_license>CC0-1.0</metadata_license>
  <project_license>MPL-2.0</project_license>
  <content_rating type="oars-1.1" />

  <developer id="org.mozilla">
    <name>Mozilla Corporation</name>
  </developer>

  <icon type="local">${DATADIR}/icons/hicolor/scalable/apps/org.mozilla.vpn.svg</icon>
  <branding>
    <color type="primary" scheme_preference="light">#f9f9fa</color>
    <color type="primary" scheme_preference="dark">#42414d</color>
  </branding>

  <releases type="external" url="https://mozilla.github.io/mozillavpn-product-details/org.mozilla.vpn.releases.xml" />

  <url type="homepage">https://vpn.mozilla.org</url>
  <url type="vcs-browser">https://github.com/mozilla-mobile/mozilla-vpn-client</url>
  <url type="help">https://support.mozilla.org/en-US/products/firefox-private-network-vpn</url>

  <supports>
    <control>pointing</control>
    <control>keyboard</control>
    <control>touch</control>
  </supports>
  <requires>
    <display_length compare="gt">640</display_length>
    <internet>always</internet>
  </requires>

  <description>
    <p>A virtual private network protects your connection to the internet, keeping your location and what you do online more private across your devices.</p>
    <p><em>Keeps your data safe on public Wi-Fi</em></p>
    <p>Log in to your bank or doctor's office from the airport, cafe or anywhere, with peace of mind.</p>
    <p><em>Blocks advertisers from targeting you</em></p>
    <p>Hide your activity from trackers and malware so you can shop without being watched.</p>
    <p><em>Helps you access global content</em></p>
    <p>Check out streaming media, websites and live-streams from other countries while you're traveling or at home.</p>
    <p><em>Features that protect your life online</em></p>
    <ul>
      <li>Connect up to 5 devices</li>
      <li>More than 500 servers in 30+ countries</li>
      <li>Fast network speeds even while gaming</li>
      <li>No logging, tracking or sharing of network data</li>
      <li>No bandwidth restrictions or throttling</li>
      <li>Extra security: whole device protection, multi-hop routing and more</li>
    </ul>
  </description>
  <project_group>Mozilla</project_group>

  <screenshots>
    <screenshot type="default">
      <caption>Mozilla VPN home screen</caption>
      <image>https://raw.githubusercontent.com/mozilla-mobile/mozilla-vpn-client/${GITREF}/docs/screenshots/linux-screenshot-home-inactive.png</image>
    </screenshot>
    <screenshot>
      <caption>Recommended servers selection screen</caption>
      <image>https://raw.githubusercontent.com/mozilla-mobile/mozilla-vpn-client/${GITREF}/docs/screenshots/linux-screenshot-selection-recommended.png</image>
    </screenshot>
    <screenshot>
      <caption>Server selection screen</caption>
      <image>https://raw.githubusercontent.com/mozilla-mobile/mozilla-vpn-client/${GITREF}/docs/screenshots/linux-screenshot-selection-multihop.png</image>
    </screenshot>
    <screenshot>
      <caption>Mozilla VPN connected to Toronto</caption>
      <image>https://raw.githubusercontent.com/mozilla-mobile/mozilla-vpn-client/${GITREF}/docs/screenshots/linux-screenshot-home-active.png</image>
    </screenshot>
    <screenshot>
      <caption>Privacy settings screen</caption>
      <image>https://raw.githubusercontent.com/mozilla-mobile/mozilla-vpn-client/${GITREF}/docs/screenshots/linux-screenshot-privacy.png</image>
    </screenshot>
  </screenshots>

  <launchable type="desktop-id">org.mozilla.vpn.desktop</launchable>
</component>
EOF
