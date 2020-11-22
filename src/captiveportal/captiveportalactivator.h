/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CAPTIVEPORTALACTIVATOR_H
#define CAPTIVEPORTALACTIVATOR_H

#include <QObject>

class CaptivePortalActivator final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(CaptivePortalActivator)

 public:
  explicit CaptivePortalActivator(QObject* parent);
  ~CaptivePortalActivator();

  void run();

 private:
  void checkStatus();
};

#endif  // CAPTIVEPORTALACTIVATOR_H
