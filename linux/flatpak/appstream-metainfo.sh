#!/bin/sh
cat << EOF
<?xml version="1.0" encoding="UTF-8"?>
<component type="desktop-application">
  <id>org.mozilla.vpn</id>

  <name>Mozilla VPN</name>
  <summary>A fast, secure and easy to use VPN. Built by the makers of Firefox</summary>
  
  <metadata_license>CC0-1.0</metadata_license>
  <project_license>MPL-2.0</project_license>
  <content_rating type="oars-1.0" />

  <developer id="org.mozilla">
    <name>Mozilla Corporation</name>
  </developer>

$(python $(dirname $0)/appstream-releases.py | sed s'/^/  /g')

  <url type="homepage">https://vpn.mozilla.org</url>
  <url type="vcs-browser">https://github.com/mozilla-mobile/mozilla-vpn-client</url>

  <supports>
    <control>pointing</control>
    <control>keyboard</control>
    <control>touch</control>
  </supports>
  
  <description>
    <p>
      A virtual private network protects your connection to the internet, keeping your location and what you do online more private across your devices.
    </p>
  </description>
  <project_group>Mozilla</project_group>
  
  <launchable type="desktop-id">org.mozilla.vpn.desktop</launchable>
</component>
EOF