/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef APPTRACKER_H
#define APPTRACKER_H

#include <QObject>
#include <QDBusConnection>
#include <QDBusInterface>

class AppTracker final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AppTracker)

 public:
  explicit AppTracker(uint userid, QDBusObjectPath path, QObject* parent);
  ~AppTracker();

 signals:
  void appLaunched(const QString& name, uint userid, int rootpid);

 private:
  uint m_userid;

 private slots:
  void gtkLaunchEvent(const QByteArray& appid, const QString& display,
                      qlonglong pid, const QStringList& uris,
                      const QVariantMap& extra);
};

#endif  // APPTRACKER_H
