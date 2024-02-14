%define _srcdir %(git rev-parse --show-toplevel)
%{!?_version: %define _version %(cat %{_srcdir}/version.txt)}

Name:      mozillavpn
Version:   %{_version}
Release:   1~git%(git log -1 --format=%h)%{?dist}
Summary:   Mozilla VPN
License:   MPL-2.0
URL:       https://vpn.mozilla.org
Packager:  Owen Kirby
Requires:  qt6-qtbase >= 6.0
Requires:  qt6-qtnetworkauth >= 6.0
Requires:  qt6-qtquickcontrols2 >= 6.0
Requires:  qt6-qtsvg >= 6.0
Requires:  qt6-qt5compat >= 6.0
Requires:  wireguard-tools

BuildRequires: cargo
BuildRequires: golang >= 1.18
BuildRequires: libcap-devel
BuildRequires: libsecret-devel
BuildRequires: openssl-devel
BuildRequires: python3-yaml
BuildRequires: qt6-qtbase-devel >= 6.0
BuildRequires: qt6-qtbase-private-devel >= 6.0
BuildRequires: qt6-qtnetworkauth-devel >= 6.0
BuildRequires: qt6-qtdeclarative-devel >= 6.0
BuildRequires: qt6-qtsvg-devel >= 6.0
BuildRequires: qt6-qttools-devel >= 6.0
BuildRequires: qt6-qtwebsockets-devel >= 6.0
BuildRequires: qt6-qt5compat-devel >= 6.0
BuildRequires: qt6-qtbase-mysql >= 6.0
BuildRequires: qt6-qtbase-odbc => 6.0
BuildRequires: qt6-qtbase-postgresql => 6.0
BuildRequires: systemd
BuildRequires: systemd-rpm-macros

%description
A fast, secure and easy to use VPN. Built by the makers of Firefox.
Read more on https://vpn.mozilla.org

%prep
%undefine _lto_cflags

%build
%define _vpath_srcdir %{_srcdir}
%cmake -DWEBEXT_INSTALL_LIBDIR=/usr/lib -DCMAKE_INSTALL_SYSCONFDIR=/etc -DBUILD_TESTING=OFF
%cmake_build

%install
%cmake_install
install -d %{buildroot}/%{_licensedir}/%{name}
install %{_srcdir}/LICENSE.md %{buildroot}/%{_licensedir}/%{name}/

%files
%license %{_licensedir}/%{name}/LICENSE.md
%{_sysconfdir}/chromium/native-messaging-hosts/mozillavpn.json
%{_sysconfdir}/opt/chrome/native-messaging-hosts/mozillavpn.json
%{_sysconfdir}/xdg/autostart/org.mozilla.vpn-startup.desktop
%{_unitdir}/mozillavpn.service
%{_bindir}/mozillavpn
%{_prefix}/lib/mozillavpn/mozillavpnnp
%{_prefix}/lib/mozilla/native-messaging-hosts/mozillavpn.json
%{_datadir}/applications/org.mozilla.vpn.desktop
%{_datadir}/dbus-1/system-services/org.mozilla.vpn.dbus.service
%{_datadir}/dbus-1/system.d/org.mozilla.vpn.conf
%{_datadir}/icons/hicolor/128x128/apps/org.mozilla.vpn.png
%{_datadir}/icons/hicolor/16x16/apps/org.mozilla.vpn.png
%{_datadir}/icons/hicolor/32x32/apps/org.mozilla.vpn.png
%{_datadir}/icons/hicolor/48x48/apps/org.mozilla.vpn.png
%{_datadir}/icons/hicolor/64x64/apps/org.mozilla.vpn.png
