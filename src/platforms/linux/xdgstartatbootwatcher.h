/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef XDGSTARTATBOOTWATCHER_H
#define XDGSTARTATBOOTWATCHER_H

#include <QObject>

class QDBusPendingCallWatcher;

class XdgStartAtBootWatcher final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(XdgStartAtBootWatcher)

 public:
  explicit XdgStartAtBootWatcher();
  ~XdgStartAtBootWatcher();

 private slots:
  void xdgResponse(uint, QVariantMap);

 private:
  void startAtBootChanged();
  QString xdgReplyPath();
  QString parentWindow();
  void callCompleted(QDBusPendingCallWatcher* call);

  QString m_replyPath;
  QString m_token;
};

#endif  // XDGSTARTATBOOTWATCHER_H
