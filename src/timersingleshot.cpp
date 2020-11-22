/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "timersingleshot.h"

#include <QTimer>

// static
void TimerSingleShot::create(QObject* parent, uint32_t timer,
                             std::function<void()>&& a_callback) {
  std::function<void()> callback = std::move(a_callback);

  QTimer* t = new QTimer(parent);

  QObject::connect(t, &QTimer::timeout, [t, callback = std::move(callback)]() {
    t->deleteLater();
    callback();
  });

  t->setSingleShot(true);
  t->start(timer);
}
