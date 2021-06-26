%define _srcdir %(git rev-parse --show-toplevel)
%define _version %(cat %{_srcdir}/version.pri | grep :VERSION | awk '{print $NF}')

Name:      mozillavpn
Version:   %{_version}
Release:   1
Summary:   Mozilla VPN
License:   MPLv2.0
URL:       https://vpn.mozilla.org
Packager:  Owen Kirby
Requires:  qt5-qtbase
Requires:  qt5-qtcharts
Requires:  qt5-qtnetworkauth
Requires:  qt5-qtquickcontrols2
Requires:  qt5-qtsvg
Requires:  wireguard-tools

BuildRequires: golang
BuildRequires: polkit-devel
BuildRequires: qt5-qtbase-devel
BuildRequires: qt5-qtcharts-devel
BuildRequires: qt5-qtnetworkauth-devel
BuildRequires: qt5-qtquickcontrols2-devel
BuildRequires: qt5-qtsvg-devel
BuildRequires: qt5-qttools-devel
BuildRequires: qt5-qtwebsockets-devel
BuildRequires: systemd-rpm-macros

%description
A fast, secure and easy to use VPN. Built by the makers of Firefox.
Read more on https://vpn.mozilla.org

%prep
%setup -q
%undefine _lto_cflags

%build
%{qmake_qt5} CONFIG+=production QT+=svg
make -j$(nproc)

%install
make install INSTALL_ROOT=%{buildroot}
install -d %{buildroot}/%{_licensedir}/%{name}
install LICENSE.md %{buildroot}/%{_licensedir}/%{name}/

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

