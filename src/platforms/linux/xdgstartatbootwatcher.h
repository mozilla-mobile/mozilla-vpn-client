/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef XDGSTARTATBOOTWATCHER_H
#define XDGSTARTATBOOTWATCHER_H

#include <QObject>

#include "xdgportal.h"

class QDBusPendingCallWatcher;

class XdgStartAtBootWatcher final : public XdgPortal {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(XdgStartAtBootWatcher)

 public:
  explicit XdgStartAtBootWatcher();
  ~XdgStartAtBootWatcher();

 private slots:
  void xdgResponse(uint, QVariantMap);

 private:
  void startAtBootChanged();
  void callCompleted(QDBusPendingCallWatcher* call);
};

#endif  // XDGSTARTATBOOTWATCHER_H
