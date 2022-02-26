%define _srcdir %(git rev-parse --show-toplevel)
%define _version %(cat %{_srcdir}/version.pri | grep :VERSION | awk '{print $NF}')

Name:      mozillavpn
Version:   %{_version}
Release:   1~git%(git log -1 --format=%h)%{?dist}
Summary:   Mozilla VPN
License:   MPLv2.0
URL:       https://vpn.mozilla.org
Packager:  Owen Kirby
Requires:  qt5-qtbase >= 5.15
Requires:  qt5-qtcharts >= 5.15
Requires:  qt5-qtnetworkauth >= 5.15
Requires:  qt5-qtquickcontrols2 >= 5.15
Requires:  qt5-qtsvg >= 5.15
Requires:  wireguard-tools

BuildRequires: golang >= 1.13
BuildRequires: polkit-devel
BuildRequires: python3-yaml
BuildRequires: qt5-qtbase-devel >= 5.15
BuildRequires: qt5-qtcharts-devel >= 5.15
BuildRequires: qt5-qtnetworkauth-devel >= 5.15
BuildRequires: qt5-qtdeclarative-devel >= 5.15
BuildRequires: qt5-qtsvg-devel >= 5.15
BuildRequires: qt5-qttools-devel >= 5.15
BuildRequires: qt5-qtwebsockets-devel >= 5.15
BuildRequires: systemd-rpm-macros

%description
A fast, secure and easy to use VPN. Built by the makers of Firefox.
Read more on https://vpn.mozilla.org

%prep
%undefine _lto_cflags

%build
%{_srcdir}/scripts/utils/import_languages.py
%{qmake_qt5} %{_srcdir}/mozillavpn.pro QT+=svg
make %{?_smp_mflags}

%install
make install INSTALL_ROOT=%{buildroot}
install -d %{buildroot}/%{_licensedir}/%{name}
install %{_srcdir}/LICENSE.md %{buildroot}/%{_licensedir}/%{name}/

%files
%license %{_licensedir}/%{name}/LICENSE.md
%{_sysconfdir}/xdg/autostart/MozillaVPN-startup.desktop
%{_unitdir}/mozillavpn.service
%{_bindir}/mozillavpn
%{_datadir}/applications/MozillaVPN.desktop
%{_datadir}/dbus-1/system-services/org.mozilla.vpn.dbus.service
%{_datadir}/dbus-1/system.d/org.mozilla.vpn.conf
%{_datadir}/icons/hicolor/128x128/apps/mozillavpn.png
%{_datadir}/icons/hicolor/16x16/apps/mozillavpn.png
%{_datadir}/icons/hicolor/32x32/apps/mozillavpn.png
%{_datadir}/icons/hicolor/48x48/apps/mozillavpn.png
%{_datadir}/icons/hicolor/64x64/apps/mozillavpn.png
%{_datadir}/polkit-1/actions/org.mozilla.vpn.policy

