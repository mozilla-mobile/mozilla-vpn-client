/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FLATPAKSTARTATBOOTWATCHER_H
#define FLATPAKSTARTATBOOTWATCHER_H

#include <QObject>

class QDBusPendingCallWatcher;

class FlatpakStartAtBootWatcher final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(FlatpakStartAtBootWatcher)

 public:
  explicit FlatpakStartAtBootWatcher();
  ~FlatpakStartAtBootWatcher();

 private slots:
  void xdgResponse(uint, QVariantMap);

 private:
  void startAtBootChanged();
  static QString xdgReplyPath();
  void callCompleted(QDBusPendingCallWatcher* call);

  QString m_replyPath;
};

#endif  // FLATPAKSTARTATBOOTWATCHER_H
