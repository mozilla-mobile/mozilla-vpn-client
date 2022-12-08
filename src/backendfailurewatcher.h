/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BACKENDFAILUREWATCHER_H
#define BACKENDFAILUREWATCHER_H

#include <QObject>

class BackendFailureWatcher final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(BackendFailureWatcher)

 public:
  explicit BackendFailureWatcher(QObject* parent);
  ~BackendFailureWatcher();

  /**
   * @brief notify all the modules that there was a backend failure.
   *
   * When all the modules are ready, the signal `readyToBackendFailure` will be
   * emitted.
   */
  void run();

 private:
  void maybeReadyToBackendFailure();

 signals:
  void readyToBackendFailure();

 private:
  int m_count = 0;
};

#endif  // BACKENDFAILUREWATCHER_H
