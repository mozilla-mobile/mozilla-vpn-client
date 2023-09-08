/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef VERSIONAPI_H
#define VERSIONAPI_H

#include "updater.h"

class VersionApi final : public Updater {
  Q_DISABLE_COPY_MOVE(VersionApi)

 public:
  VersionApi(QObject* parent);
  ~VersionApi();

  void start(Task* task) override;

 private:
  [[nodiscard]] bool processData(const QByteArray& data);
};

#endif  // VERSIONAPI_H
