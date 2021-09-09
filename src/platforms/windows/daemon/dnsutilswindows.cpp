/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "dnsutilswindows.h"
#include "leakdetector.h"
#include "logger.h"

#include <QScopeGuard>

#include <windows.h>
#include <iphlpapi.h>

namespace {
Logger logger(LOG_WINDOWS, "DnsUtilsWindows");
}

DnsUtilsWindows::DnsUtilsWindows(QObject* parent) : DnsUtils(parent) {
  MVPN_COUNT_CTOR(DnsUtilsWindows);

  logger.debug() << "DnsUtilsWindows created.";
}

DnsUtilsWindows::~DnsUtilsWindows() {
  MVPN_COUNT_DTOR(DnsUtilsWindows);
  restoreResolvers();
  logger.debug() << "DnsUtilsWindows destroyed.";
}

bool DnsUtilsWindows::updateResolvers(const QString& ifname,
                                      const QList<QHostAddress>& resolvers) {

  // Lookup the interface GUID
  NET_LUID luid;
  GUID guid;
  if (ConvertInterfaceAliasToLuid((wchar_t*)ifname.utf16(), &luid) != 0) {
    logger.error() << "Failed to resolved LUID for" << ifname;
    return false;
  }
  if (ConvertInterfaceLuidToGuid(&luid, &guid) != 0) {
    logger.error() << "Failed to resolved GUID for" << ifname;
    return false;
  }
  m_ifname = ifname;

  QStringList v4resolvers;
  QStringList v6resolvers;
  for (const QHostAddress& addr : resolvers) {
    if (addr.protocol() == QAbstractSocket::IPv4Protocol) {
      v4resolvers.append(addr.toString());
    }
    if (addr.protocol() == QAbstractSocket::IPv6Protocol) {
      v6resolvers.append(addr.toString());
    }
  }

  DNS_INTERFACE_SETTINGS settings;
  settings.Version = DNS_INTERFACE_SETTINGS_VERSION1;
  settings.Flags = DNS_SETTING_NAMESERVER | DNS_SETTING_SEARCHLIST;
  settings.Domain = nullptr;
  settings.NameServer = nullptr;
  settings.SearchList = (wchar_t*)L".";
  settings.RegistrationEnabled = false;
  settings.RegisterAdapterName = false;
  settings.EnableLLMNR = false;
  settings.QueryAdapterName = false;
  settings.ProfileNameServer = nullptr;

  // Configure nameservers for IPv4
  QString v4resolverstring = v4resolvers.join(",");
  settings.NameServer = (wchar_t*)v4resolverstring.utf16();
  DWORD v4result = SetInterfaceDnsSettings(guid, &settings);
  if (v4result != NO_ERROR) {
    logger.error() << "Failed to configure IPv4 resolvers:" << v4result;
  }

  // Configure nameservers for IPv6
  QString v6resolverstring = v6resolvers.join(",");
  settings.Flags |= DNS_SETTING_IPV6;
  settings.NameServer = (wchar_t*)v6resolverstring.utf16();
  DWORD v6result = SetInterfaceDnsSettings(guid, &settings);
  if (v6result != NO_ERROR) {
    logger.error() << "Failed to configure IPv6 resolvers" << v6result;
  }

  return ((v4result == NO_ERROR) || (v6result == NO_ERROR));
}

bool DnsUtilsWindows::restoreResolvers() {
  if (!m_ifname.isEmpty()) {
    QList<QHostAddress> empty;
    updateResolvers(m_ifname, empty);
  }
  return true;
}
