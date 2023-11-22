/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "macosdnsmanager.h"

#include <systemconfiguration/scdynamicstore.h>
#include <systemconfiguration/scpreferences.h>
#include <systemconfiguration/systemconfiguration.h>

#include <QCoreApplication>
#include <QHostAddress>
#include <QScopeGuard>
#include <QTextStream>

#include "commandlineparser.h"
#include "constants.h"
#include "leakdetector.h"
#include "signalhandler.h"

MacOSDnsManager::MacOSDnsManager(QObject* parent)
    : Command(parent, "macosdnsmanager", "Update the DNS settings while the VPN is active") {
  MZ_COUNT_CTOR(MacOSDnsManager);
}

MacOSDnsManager::~MacOSDnsManager() {
  if (m_scStore != nullptr) {
    restoreSnapshot();
    CFRelease(m_scStore);
  }
  MZ_COUNT_DTOR(MacOSDnsManager);
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

int MacOSDnsManager::run(QStringList& tokens) {
  Q_ASSERT(!tokens.isEmpty());
  QString appName = tokens[0];

  QCoreApplication app(CommandLineParser::argc(), CommandLineParser::argv());

  QCoreApplication::setApplicationName("Mozilla VPN DNS Manager");
  QCoreApplication::setApplicationVersion(Constants::versionString());
  QTextStream stream(stderr);

  if (tokens.length() <= 1) {
    stream << "Error: At least one DNS server address is required." << Qt::endl;
    stream << Qt::endl;
    stream << "Usage: [options] ADDR [ADDR ...]" << Qt::endl;
    return 1;
  }

  // Ensure that the DNS addresses we received are valid.
  QStringList dnsAddrList = tokens.mid(1);
  for (const QString& addr : dnsAddrList) {
    QHostAddress host = QHostAddress(addr);
    if (host.isNull()) {
      stream << "Error: DNS Server \'" << addr << "\' is invalid." << Qt::endl;
      return 1;
    }
    if (host.isBroadcast() || host.isMulticast()) {
      stream << "Error: DNS Server \'" << addr << "\' is a multicast address."
             << Qt::endl;
      return 1;
    }
    // TODO: Should we prohibit the loopback address?
  }

  // Prepare the updated DNS configuration.
  CFMutableDictionaryRef dnsConfig = CFDictionaryCreateMutable(
      kCFAllocatorSystemDefault, 0, &kCFCopyStringDictionaryKeyCallBacks,
      &kCFTypeDictionaryValueCallBacks);
  auto configGuard = qScopeGuard([&] { CFRelease(dnsConfig); });
  cfDictSetStringList(dnsConfig, kSCPropNetDNSServerAddresses, tokens.mid(1));
  cfDictSetString(dnsConfig, kSCPropNetDNSDomainName, "lan");

  // Open the system configuration store.
  m_scStore = SCDynamicStoreCreate(kCFAllocatorSystemDefault,
                                  CFSTR("mozillavpn"), nullptr, nullptr);
  if (m_scStore == nullptr) {
    stream << "Failed to open system configuration:" << scErrorMessage()
           << Qt::endl;
    return 1;
  }

  // Get the list of current network services and take a snapshot.
  for (const QString& uuid : enumerateNetServices()) {
    stream << "Found network service: " << uuid << Qt::endl;
    CFStringRef dnsPath = CFStringCreateWithFormat(
        kCFAllocatorSystemDefault, nullptr,
        CFSTR("Setup:/Network/Service/%s/DNS"), qPrintable(uuid));
    auto pathGuard = qScopeGuard([&] { CFRelease(dnsPath); });

    // Take a snapshot of the existing configuration.
    m_snapshot[uuid] =
        (CFDictionaryRef)SCDynamicStoreCopyValue(m_scStore, dnsPath);

    // Install the updated configuration.
    if (!SCDynamicStoreSetValue(m_scStore, dnsPath, dnsConfig)) {
      stream << "Failed to update configuration for " << uuid << Qt::endl;
      stream << "Error: " << scErrorMessage() << Qt::endl;
      return 1;
    }
  }

  // Block this process and wait for a termination signal.
  stream << "Waiting for termination signal." << Qt::endl;
  int sig = waitForTermination();
  stream << "Caught signal " << sig << ". Preparing to exit." << Qt::endl;

  // If we exit on SIGHUP, leave the DNS configuration in-place.
  if (sig == SIGHUP) {
    clearSnapshot();
  }

  // The DNS configuration should be resored by the destructor.
  return 0;
}

QStringList MacOSDnsManager::enumerateNetServices() {
  CFArrayRef netServices = SCDynamicStoreCopyKeyList(
      m_scStore, CFSTR("Setup:/Network/Service/[0-9A-F-]+"));
  if (netServices == nullptr) {
    return QStringList();
  }
  auto serviceGuard = qScopeGuard([&] { CFRelease(netServices); });

  QStringList result;
  for (CFIndex i = 0; i < CFArrayGetCount(netServices); i++) {
    QString service = cfParseString(CFArrayGetValueAtIndex(netServices, i));
    QString uuid = service.section('/', 3, 3);
    if (!uuid.isEmpty()) {
      result.append(uuid);
    }
  }

  return result;
}

QString MacOSDnsManager::scErrorMessage() {
  int error = SCError();
  return QString(SCErrorString(error));
}

void MacOSDnsManager::restoreSnapshot(void) {
  Q_ASSERT(m_scStore != nullptr);

  for (const QString& uuid : m_snapshot.keys()) {
    CFStringRef dnsPath = CFStringCreateWithFormat(
        kCFAllocatorSystemDefault, nullptr,
        CFSTR("Setup:/Network/Service/%s/DNS"), qPrintable(uuid));

    // Restore the snapshotted DNS configuration for this service.
    CFDictionaryRef dnsConfig = m_snapshot.take(uuid);
    if (dnsConfig) {
      SCDynamicStoreSetValue(m_scStore, dnsPath, dnsConfig);
      CFRelease(dnsConfig);
    } else {
      SCDynamicStoreRemoveValue(m_scStore, dnsPath);
    }
    CFRelease(dnsPath);
  }
}

// Clear the DNS snapshot without doing anything.
void MacOSDnsManager::clearSnapshot(void) {
  for (CFDictionaryRef dnsConfig : m_snapshot) {
    if (dnsConfig) {
      CFRelease(dnsConfig);
    }
  }
  m_snapshot.clear();
}

// Block the process and wait for a termination signal.
// TODO: Also check for exit of the parent process.
int MacOSDnsManager::waitForTermination(void) {
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
    return SIGKILL;
  }
  return sig;
}

static Command::RegistrationProxy<MacOSDnsManager> s_commandMacOSDaemon;
