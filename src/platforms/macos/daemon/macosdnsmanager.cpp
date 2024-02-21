/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "macosdnsmanager.h"

#include <sys/event.h>
#include <systemconfiguration/scdynamicstore.h>
#include <systemconfiguration/scpreferences.h>
#include <systemconfiguration/systemconfiguration.h>

#include <QCoreApplication>
#include <QHostAddress>
#include <QScopeGuard>
#include <QTextStream>

#include "commandlineparser.h"
#include "context/constants.h"
#include "signalhandler.h"
#include "utilities/leakdetector.h"

MacOSDnsManager::MacOSDnsManager(QObject* parent)
    : Command(parent, "macosdnsmanager",
              "Update the DNS settings while the VPN is active") {
  MZ_COUNT_CTOR(MacOSDnsManager);
}

MacOSDnsManager::~MacOSDnsManager() {
  if (m_scStore != nullptr) {
    restoreSnapshot();
    CFRelease(m_scStore);
  }
  MZ_COUNT_DTOR(MacOSDnsManager);
}

int MacOSDnsManager::run(QStringList& tokens) {
  Q_ASSERT(!tokens.isEmpty());
  const QString appName = tokens[0];

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
  const QStringList dnsAddrList = tokens.mid(1);
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
  }

  // Prepare the updated DNS configuration.
  const CFMutableDictionaryRef dnsConfig = CFDictionaryCreateMutable(
      kCFAllocatorSystemDefault, 0, &kCFCopyStringDictionaryKeyCallBacks,
      &kCFTypeDictionaryValueCallBacks);
  auto configGuard = qScopeGuard([&] { CFRelease(dnsConfig); });
  cfDictSetStringList(dnsConfig, kSCPropNetDNSServerAddresses, dnsAddrList);
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

  // SIGHUP is typically used to inform processes of a configuration change.
  // Handle this for now by exiting without restoring the DNS resolver config.
  if (sig == SIGHUP) {
    clearSnapshot();
  }

  // The DNS configuration should be resored by the destructor.
  return 0;
}

QStringList MacOSDnsManager::enumerateNetServices() {
  CFArrayRef netServices = SCDynamicStoreCopyKeyList(
      m_scStore, CFSTR("Setup:/Network/Service/[0-9A-Fa-f-]+"));
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
int MacOSDnsManager::waitForTermination(void) {
  int kq = kqueue();

  // Watch for NOTE_EXIT on our parent process.
  // https://developer.apple.com/library/archive/technotes/tn2050/_index.html
  struct kevent changes;
  EV_SET(&changes, getppid(), EVFILT_PROC, EV_ADD | EV_RECEIPT, NOTE_EXIT, 0,
         nullptr);
  kevent(kq, &changes, 1, &changes, 1, nullptr);

  // Prepare to handle termination signals.
  static int sig = 0;
  signal(SIGINT, [](int s) { sig = s; });
  signal(SIGTERM, [](int s) { sig = s; });
  signal(SIGHUP, [](int s) { sig = s; });
  signal(SIGPIPE, [](int s) { sig = s; });

  struct kevent ev;
  int ret = kevent(kq, nullptr, 0, &ev, 1, nullptr);
  close(kq);
  if ((ret < 0) && (errno == EINTR)) {
    // Received a signal.
    return sig;
  } else if ((ret <= 0) || (ev.flags & EV_ERROR)) {
    // Something went horribly wrong.
    return SIGKILL;
  } else {
    // Received NOTE_EXIT; our parent has terminated.
    return SIGCHLD;
  }
}

// static
QString MacOSDnsManager::cfParseString(CFTypeRef ref) {
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

// static
void MacOSDnsManager::cfDictSetString(CFMutableDictionaryRef dict,
                                      CFStringRef name, const QString& value) {
  if (value.isNull()) {
    return;
  }
  CFStringRef cfValue = CFStringCreateWithCString(
      kCFAllocatorSystemDefault, qUtf8Printable(value), kCFStringEncodingUTF8);
  CFDictionarySetValue(dict, name, cfValue);
  CFRelease(cfValue);
}

// static
void MacOSDnsManager::cfDictSetStringList(CFMutableDictionaryRef dict,
                                          CFStringRef name,
                                          const QStringList& valueList) {
  if (valueList.isEmpty()) {
    return;
  }

  CFMutableArrayRef array = CFArrayCreateMutable(kCFAllocatorSystemDefault, 0,
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

static Command::RegistrationProxy<MacOSDnsManager> s_commandMacOSDaemon;
