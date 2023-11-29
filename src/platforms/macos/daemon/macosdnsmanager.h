/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MACOSDNSMANAGER_H
#define MACOSDNSMANAGER_H

#include <systemconfiguration/scdynamicstore.h>

#include <QMap>

#include "command.h"

// This class adds a command to the Mozilla VPN that overwrites the system
// DNS configuration, and restores it upon program exit. When run in a separate
// process, this provides a watchdog mechanism to ensure that no matter how the
// daemon terminates (or crashes!) we always do our best to clean up.
//
// This command accepts DNS server addresses as positional arguments to be
// applied to the system. For example:
//   Mozilla\ VPN macosdnsmanager 8.8.8.8
//
// This program will exit upon the normal POSIX termination signals (eg: SIGINT
// SIGTERM, and so on), or when it detects that the parent process has exited
// via kqueue().
//
// To exit without restoring the DNS configuration, this program can be sent
// the hang-up signal (SIGHUP).
class MacOSDnsManager final : public Command {
 public:
  explicit MacOSDnsManager(QObject* parent);
  ~MacOSDnsManager();

  int run(QStringList& tokens) override;

 private:
  QStringList enumerateNetServices();
  static QString scErrorMessage();
  void restoreSnapshot();
  void clearSnapshot();
  int waitForTermination();

  // Core foundation type handling helpers.
  static QString cfParseString(CFTypeRef ref);
  static void cfDictSetString(CFMutableDictionaryRef dict, CFStringRef name,
                              const QString& value);
  static void cfDictSetStringList(CFMutableDictionaryRef dict, CFStringRef name,
                                  const QStringList& valueList);

 private:
  SCDynamicStoreRef m_scStore = nullptr;
  QMap<QString, CFDictionaryRef> m_snapshot;
};

#endif  // MACOSDNSMANAGER_H
