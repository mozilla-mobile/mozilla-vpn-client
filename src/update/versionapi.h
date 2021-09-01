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

  void start() override;

  // compare 2 version strings and return:
  // - -1 if the first one is lower than the second one or if the second one is
  // empty.
  // - 0 if they are equal
  // - 1 if the first one is greater than the second one or if the first one is
  // empty.
  static int compareVersions(const QString& a, const QString& b);

  // Strips the minor version
  // e.g 2.2.2 -> 2.2.0
  static QString stripMinor(const QString& a);

 private:
  [[nodiscard]] bool processData(const QByteArray& data);
};

#endif  // VERSIONAPI_H
