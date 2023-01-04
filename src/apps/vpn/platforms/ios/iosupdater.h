/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef IOSUPDATER_H
#define IOSUPDATER_H

#include "updater.h"
#include "versionapi.h"

class IOSUpdater final : public Updater {
  Q_DISABLE_COPY_MOVE(IOSUpdater)

 public:
  explicit IOSUpdater(QObject* parent);
  ~IOSUpdater();

  void start(Task* task) override;

 private:
  [[nodiscard]] bool processData(const QString& latestVersion,
                                 const QByteArray& data);

  void checkVersion(Task* task, const QString& lastVersion);

 private:
  VersionApi* m_versionApi = nullptr;
  bool m_updateRequired = false;
};

#endif  // IOSUPDATER_H
