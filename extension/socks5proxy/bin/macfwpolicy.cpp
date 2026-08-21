/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "macfwpolicy.h"

extern "C" {
#include <CoreFoundation/CoreFoundation.h>
};

#include <QtDebug>
#include <QLocalServer>
#include <QProcessEnvironment>

#include "proxyconnection.h"
#include "socks5.h"

MacFwPolicy::MacFwPolicy(Socks5* proxy) : QObject(proxy) {
  // For QLocalServers, we should check that the local client
  // name corresponds to a known web browser.
  if (qobject_cast<QLocalServer*>(proxy->parent()) != nullptr) {
    connect(proxy, &Socks5::incomingConnection, this,
            &MacFwPolicy::checkLocalSocket);
  }

  // Search for applications with the NSUserActivityTypes containing
  // NSUserActivityTypeBrowsingWeb, indicating that they are web browsers.
  QProcessEnvironment pe = QProcessEnvironment::systemEnvironment();
  if (pe.contains("HOME")) {
    enumerateBrowsers(pe.value("HOME") + "/Applications");
    enumerateBrowsers(pe.value("HOME") + "/Applications/Utilities");
  }

  enumerateBrowsers("/Applications");
  enumerateBrowsers("/Applications/Utilities");
  enumerateBrowsers("/System/Applications");
  enumerateBrowsers("/System/Applications/Utilities");
  enumerateBrowsers("/System/Cryptexes/App/System/Applications");
}

void MacFwPolicy::enumerateBrowsers(const QString& appDir) {
  static const CFStringRef kBundleTypeApp = CFSTR(".app");
  static const CFStringRef kActivityTypes = CFSTR("NSUserActivityTypes");
  static const CFStringRef kActivityBrowser = CFSTR("NSUserActivityTypeBrowsingWeb");

  CFStringRef s = appDir.toCFString();
  CFURLRef url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, s,
                                               kCFURLPOSIXPathStyle, TRUE);
  CFArrayRef list = CFBundleCreateBundlesFromDirectory(kCFAllocatorDefault, url,
                                                       kBundleTypeApp);
  for (CFIndex i = 0; list && i < CFArrayGetCount(list); i++) {
    CFBundleRef bundle = (CFBundleRef)CFArrayGetValueAtIndex(list, i);
    CFArrayRef values =
        (CFArrayRef)CFBundleGetValueForInfoDictionaryKey(bundle, kActivityTypes);
    if ((!values) || (CFGetTypeID(values) != CFArrayGetTypeID())) {
      continue;
    }
    CFRange range = {
        .location = 0,
        .length = CFArrayGetCount(values),
    };
    if (CFArrayContainsValue(values, range, kActivityBrowser)) {
        QString id = QString::fromCFString(CFBundleGetIdentifier(bundle));
        qDebug() << "Authorizing browser:" << id;
        m_browsers.append(id);
    }
  }
  CFRelease(url);
  CFRelease(list);
  CFRelease(s);
}

void MacFwPolicy::checkLocalSocket(ProxyConnection* connection) {
  if (!m_browsers.contains(connection->clientName())) {
    connection->abort();
  }
}
