#!/usr/bin/make -f

export DH_VERBOSE=1

DEB_HOST_MULTIARCH ?= $(shell dpkg-architecture -qDEB_HOST_MULTIARCH)
DEB_VERSION ?= $(shell dpkg-parsechangelog -SVersion)

%:
	dh $@ --with=systemd --warn-missing

override_dh_auto_configure:
	python3 scripts/importLanguages.py
	qmake CONFIG-=debug CONFIG+=release CONFIG-=debug_and_release BUILD_ID=$(DEB_VERSION) CONFIG+=webextension

override_dh_installdocs:

override_dh_installinfo:

override_dh_installsystemd:
	dh_installsystemd linux/mozillavpn.service

override_dh_systemd_start:
	dh_systemd_start linux/mozillavpn.service

override_dh_systemd_enable:
	dh_systemd_enable linux/mozillavpn.service

