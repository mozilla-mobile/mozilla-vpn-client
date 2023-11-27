/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MACOSDNSMANAGER_H
#define MACOSDNSMANAGER_H

#include <systemconfiguration/scdynamicstore.h>

#include <QMap>

#include "command.h"

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
