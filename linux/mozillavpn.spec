%define _srcdir %(git rev-parse --show-toplevel)
%define _version %(cat %{_srcdir}/version.pri | grep :VERSION | awk '{print $NF}')

Name:      mozillavpn
Version:   %{_version}
Release:   1~git%(git log -1 --format=%h)%{?dist}
Summary:   Mozilla VPN
License:   MPLv2.0
URL:       https://vpn.mozilla.org
Packager:  Owen Kirby
Requires:  qt6-qtbase >= 6.2.0
Requires:  qt6-qt5compat >= 6.2.0
Requires:  qt6-qtcharts >= 6.2.0
Requires:  qt6-qtnetworkauth >= 6.2.0
Requires:  qt6-qtquickcontrols2 >= 6.2.0
Requires:  qt6-qtsvg >= 6.2.0
Requires:  wireguard-tools

BuildRequires: golang >= 1.13
BuildRequires: polkit-devel
BuildRequires: python3-yaml
BuildRequires: cargo
BuildRequires: qt6-qtbase-devel >= 6.2.0
BuildRequires: qt6-qt5compat-devel >= 6.2.0
BuildRequires: qt6-qtcharts-devel >= 6.2.0
BuildRequires: qt6-qtnetworkauth-devel >= 6.2.0
BuildRequires: qt6-qtdeclarative-devel >= 6.2.0
BuildRequires: qt6-qtsvg-devel >= 6.2.0
BuildRequires: qt6-qttools-devel >= 6.2.0
BuildRequires: qt6-qtwebsockets-devel >= 6.2.0
BuildRequires: systemd-rpm-macros

%description
A fast, secure and easy to use VPN. Built by the makers of Firefox.
Read more on https://vpn.mozilla.org

%prep
%undefine _lto_cflags

%build
%{_srcdir}/scripts/utils/import_languages.py
%{qmake_qt6} %{_srcdir}/mozillavpn.pro QT+=svg
make %{?_smp_mflags}

%install
make install INSTALL_ROOT=%{buildroot}
install -d %{buildroot}/%{_licensedir}/%{name}
install %{_srcdir}/LICENSE.md %{buildroot}/%{_licensedir}/%{name}/

%files
%license %{_licensedir}/%{name}/LICENSE.md
%{_sysconfdir}/chromium/native-messaging-hosts/mozillavpn.json
%{_sysconfdir}/opt/chrome/native-messaging-hosts/mozillavpn.json
%{_sysconfdir}/xdg/autostart/MozillaVPN-startup.desktop
%{_unitdir}/mozillavpn.service
%{_bindir}/mozillavpn
%{_prefix}/lib/mozillavpn/mozillavpnnp
%{_prefix}/lib/mozilla/native-messaging-hosts/mozillavpn.json
%{_datadir}/applications/MozillaVPN.desktop
%{_datadir}/dbus-1/system-services/org.mozilla.vpn.dbus.service
%{_datadir}/dbus-1/system.d/org.mozilla.vpn.conf
%{_datadir}/icons/hicolor/128x128/apps/mozillavpn.png
%{_datadir}/icons/hicolor/16x16/apps/mozillavpn.png
%{_datadir}/icons/hicolor/32x32/apps/mozillavpn.png
%{_datadir}/icons/hicolor/48x48/apps/mozillavpn.png
%{_datadir}/icons/hicolor/64x64/apps/mozillavpn.png
%{_datadir}/polkit-1/actions/org.mozilla.vpn.policy
