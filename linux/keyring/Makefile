PREFIX ?= /usr

build:

install:
	install -d $(DESTDIR)$(PREFIX)/share/keyrings
	install -m 644 mozilla-apt-keyring.gpg $(DESTDIR)/$(PREFIX)/share/keyrings/
	install -d $(DESTDIR)/etc/apt/sources.list.d
	install -m 644 mozilla-apt-repo.list $(DESTDIR)/etc/apt/sources.list.d/
	install -d $(DESTDIR)/etc/apt/preferences.d
	install -m 644 mozilla-vpn.pref $(DESTDIR)/etc/apt/preferences.d/

deb:
	dpkg-buildpackage -b -us -uc

.PHONY: build install deb

