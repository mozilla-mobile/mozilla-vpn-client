/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "dnsutilsmacos.h"

#include <systemconfiguration/scdynamicstore.h>
#include <systemconfiguration/scpreferences.h>

#include <QScopeGuard>

#include "leakdetector.h"
#include "logger.h"

namespace {
Logger logger("DnsUtilsMacos");
}

DnsUtilsMacos::DnsUtilsMacos(QObject* parent) : DnsUtils(parent) {
  MZ_COUNT_CTOR(DnsUtilsMacos);

  m_scStore = SCDynamicStoreCreate(kCFAllocatorSystemDefault,
                                   CFSTR("mozillavpn"), nullptr, nullptr);
  if (m_scStore == nullptr) {
    logger.error() << "Failed to create system configuration store ref";
  }

  logger.debug() << "DnsUtilsMacos created.";
}

DnsUtilsMacos::~DnsUtilsMacos() {
  MZ_COUNT_DTOR(DnsUtilsMacos);
  restoreResolvers();
  logger.debug() << "DnsUtilsMacos destroyed.";
}

static QString cfParseString(CFTypeRef ref) {
  if (CFGetTypeID(ref) != CFStringGetTypeID()) {
    return QString();
  }

  CFStringRef stringref = (CFStringRef)ref;
  CFRange range;
  range.location = 0;
  range.length = CFStringGetLength(stringref);
  if (range.length <= 0) {
    return QString();
  }

  UniChar* buf = (UniChar*)malloc(range.length * sizeof(UniChar));
  if (!buf) {
    return QString();
  }
  auto guard = qScopeGuard([&] { free(buf); });

  CFStringGetCharacters(stringref, range, buf);
  return QString::fromUtf16(buf, range.length);
}

static QStringList cfParseStringList(CFTypeRef ref) {
  if (CFGetTypeID(ref) != CFArrayGetTypeID()) {
    return QStringList();
  }

  CFArrayRef array = (CFArrayRef)ref;
  QStringList result;
  for (CFIndex i = 0; i < CFArrayGetCount(array); i++) {
    CFTypeRef value = CFArrayGetValueAtIndex(array, i);
    result.append(cfParseString(value));
  }

  return result;
}

static void cfDictSetString(CFMutableDictionaryRef dict, CFStringRef name,
                            const QString& value) {
  if (value.isNull()) {
    return;
  }
  CFStringRef cfValue = CFStringCreateWithCString(
      kCFAllocatorSystemDefault, qUtf8Printable(value), kCFStringEncodingUTF8);
  CFDictionarySetValue(dict, name, cfValue);
  CFRelease(cfValue);
}

static void cfDictSetStringList(CFMutableDictionaryRef dict, CFStringRef name,
                                const QStringList& valueList) {
  if (valueList.isEmpty()) {
    return;
  }

  CFMutableArrayRef array;
  array = CFArrayCreateMutable(kCFAllocatorSystemDefault, 0,
                               &kCFTypeArrayCallBacks);
  if (array == nullptr) {
    return;
  }

  for (const QString& rstring : valueList) {
    CFStringRef cfAddr = CFStringCreateWithCString(kCFAllocatorSystemDefault,
                                                   qUtf8Printable(rstring),
                                                   kCFStringEncodingUTF8);
    CFArrayAppendValue(array, cfAddr);
    CFRelease(cfAddr);
  }
  CFDictionarySetValue(dict, name, array);
  CFRelease(array);
}

bool DnsUtilsMacos::updateResolvers(const QString& ifname,
                                    const QList<QHostAddress>& resolvers) {
  Q_UNUSED(ifname);

  // Prepare the DNS configuration.
  CFMutableDictionaryRef dnsConfig = CFDictionaryCreateMutable(
      kCFAllocatorSystemDefault, 0, &kCFCopyStringDictionaryKeyCallBacks,
      &kCFTypeDictionaryValueCallBacks);
  auto configGuard = qScopeGuard([&] { CFRelease(dnsConfig); });
  QStringList list;
  for (const QHostAddress& addr : resolvers) {
    list.append(addr.toString());
  }
  cfDictSetStringList(dnsConfig, kSCPropNetDNSServerAddresses, list);
  cfDictSetString(dnsConfig, kSCPropNetDNSDomainName, "lan");

  // Take a snapshot of the DNS services, and start a process to restore it.
  if (m_dnsSnapshotPid < 0) {
    if (!takeDnsSnapshot()) {
      return false;
    }
  }

  // Get the list of current network services.
  CFArrayRef netServices = SCDynamicStoreCopyKeyList(
      m_scStore, CFSTR("Setup:/Network/Service/[0-9A-F-]+"));
  if (netServices == nullptr) {
    return false;
  }
  auto serviceGuard = qScopeGuard([&] { CFRelease(netServices); });

  // Configure each network service to use our DNS.
  for (CFIndex i = 0; i < CFArrayGetCount(netServices); i++) {
    QString service = cfParseString(CFArrayGetValueAtIndex(netServices, i));
    QString uuid = service.section('/', 3, 3);
    if (uuid.isEmpty()) {
      continue;
    }

    logger.debug() << "Setting DNS config for" << uuid;
    CFStringRef dnsPath = CFStringCreateWithFormat(
        kCFAllocatorSystemDefault, nullptr,
        CFSTR("Setup:/Network/Service/%s/DNS"), qPrintable(uuid));
    if (!dnsPath) {
      continue;
    }
    SCDynamicStoreSetValue(m_scStore, dnsPath, dnsConfig);
    CFRelease(dnsPath);
  }

  return true;
}

bool DnsUtilsMacos::restoreResolvers() {
  if (m_dnsSnapshotPid < 0) {
    logger.debug() << "No DNS resolvers to restore";
    return false;
  }

  // Kill the DNS watchdog to restore the resolver configuration.
  auto guard = qScopeGuard([&] { m_dnsSnapshotPid = -1; });
  if (kill(m_dnsSnapshotPid, SIGINT) < 0) {
    logger.debug() << "Failed to signal DNS restorer";
    return false;
  }

  // Wait for the process to exit, and gather it's status.
  int status = 0;
  if (waitpid(m_dnsSnapshotPid, &status, 0) < 0) {
    logger.debug() << "Failed to receive child status";
  } else if (!WIFEXITED(status)) {
    logger.error() << "Failed to terminate DNS restorer";
  } else {
    logger.info() << "DNS restorer completed:" << strerror(WEXITSTATUS(status));
  }
  return true;
}

DnsUtilsMacos::DnsBackup DnsUtilsMacos::backupService(const QString& uuid) {
  DnsBackup backup;
  CFStringRef path = CFStringCreateWithFormat(
      kCFAllocatorSystemDefault, nullptr,
      CFSTR("Setup:/Network/Service/%s/DNS"), qPrintable(uuid));
  CFDictionaryRef config =
      (CFDictionaryRef)SCDynamicStoreCopyValue(m_scStore, path);
  auto serviceGuard = qScopeGuard([&] {
    if (config) {
      CFRelease(config);
    }
    CFRelease(path);
  });

  // Parse the DNS protocol entry and save it for later.
  if (config) {
    CFTypeRef value;
    value = CFDictionaryGetValue(config, kSCPropNetDNSDomainName);
    if (value) {
      backup.m_domain = cfParseString(value);
    }
    value = CFDictionaryGetValue(config, kSCPropNetDNSServerAddresses);
    if (value) {
      backup.m_servers = cfParseStringList(value);
    }
    value = CFDictionaryGetValue(config, kSCPropNetDNSSearchDomains);
    if (value) {
      backup.m_search = cfParseStringList(value);
    }
    value = CFDictionaryGetValue(config, kSCPropNetDNSSortList);
    if (value) {
      backup.m_sortlist = cfParseStringList(value);
    }
  }

  return backup;
}

// Fork a child process to handle restoration of the DNS configuration.
// This ensures that we always cleanup after ourselves, even if the
// daemon happens to crash.
bool DnsUtilsMacos::takeDnsSnapshot(void) {
  // Get the list of current network services.
  CFArrayRef netServices = SCDynamicStoreCopyKeyList(
      m_scStore, CFSTR("Setup:/Network/Service/[0-9A-F-]+"));
  if (netServices == nullptr) {
    return false;
  }

  // Take a snapshot of the DNS configuration.
  QMap<QString, DnsBackup> snapshot;
  for (CFIndex i = 0; i < CFArrayGetCount(netServices); i++) {
    QString service = cfParseString(CFArrayGetValueAtIndex(netServices, i));
    QString uuid = service.section('/', 3, 3);
    if (uuid.isEmpty()) {
      continue;
    }
    snapshot[uuid] = backupService(uuid);
  }
  CFRelease(netServices);

  // Fork the process. The parent/daemon continues on doing daemon stuff.
  int pid = fork();
  if (pid < 0) {
    logger.error() << "Failed to start DNS restoration watchdog";
    return false;
  }
  if (pid > 0) {
    m_dnsSnapshotPid = pid;
    return true;
  }

  // If we get here, we are the child. Do nothing until the interface goes down.
  int sig;
  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set, SIGINT);
  sigaddset(&set, SIGTERM);
  sigaddset(&set, SIGHUP);
  signal(SIGINT, [](int s) { Q_UNUSED(s); });
  signal(SIGTERM, [](int s) { Q_UNUSED(s); });
  signal(SIGHUP, [](int s) { Q_UNUSED(s); });
  if (sigwait(&set, &sig) != 0) {
    // Something went horribly wrong.
    exit(errno);
  }
  // If we receive a SIGHUP, then exit without restoring anything.
  if (sig == SIGHUP) {
    exit(0);
  }

  // Restore the snapshot of the DNS configuration.
  SCDynamicStoreRef store =
      SCDynamicStoreCreate(kCFAllocatorSystemDefault,
                           CFSTR("mozillavpn-restorer"), nullptr, nullptr);

  for (const QString& uuid : snapshot.keys()) {
    CFStringRef path = CFStringCreateWithFormat(
        kCFAllocatorSystemDefault, nullptr,
        CFSTR("Setup:/Network/Service/%s/DNS"), qPrintable(uuid));

    const DnsBackup& entry = snapshot[uuid];
    if (entry.isValid()) {
      CFMutableDictionaryRef config;
      config = CFDictionaryCreateMutable(kCFAllocatorSystemDefault, 0,
                                         &kCFCopyStringDictionaryKeyCallBacks,
                                         &kCFTypeDictionaryValueCallBacks);

      cfDictSetString(config, kSCPropNetDNSDomainName, entry.m_domain);
      cfDictSetStringList(config, kSCPropNetDNSSearchDomains, entry.m_search);
      cfDictSetStringList(config, kSCPropNetDNSServerAddresses,
                          entry.m_servers);
      cfDictSetStringList(config, kSCPropNetDNSSortList, entry.m_sortlist);
      SCDynamicStoreSetValue(store, path, config);
      CFRelease(config);
    } else {
      SCDynamicStoreRemoveValue(store, path);
    }
    CFRelease(path);
  }

  // Job is done.
  exit(0);
  return false;
}
