/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADDONMANAGER_H
#define ADDONMANAGER_H

#include "addon.h"

#include <QHash>
#include <QObject>

class AddonManager final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AddonManager)

 public:
  static AddonManager* instance();

  ~AddonManager();

  bool load(const QString& addonFileName);
  void unload(const QString& addonName);
  void run(const QString& addonName);

  void retranslate();

 private:
  explicit AddonManager(QObject* parent);

 signals:
  void runAddon(Addon* addon);
  void unloadAddon(const QString& addonId);

 private:
  QHash<QString, Addon*> m_addons;
};

#endif  // ADDONMANAGER_H
