/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef STARTUPNOTIFYWATCHER_H
#define STARTUPNOTIFYWATCHER_H

#include <QMap>
#include <QObject>
#include <QSocketNotifier>
#include <QString>

#include <xcb/xcb.h>

class StartupNotifyWatcher : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(StartupNotifyWatcher)

 public:
  explicit StartupNotifyWatcher(const QString& display, QObject* parent);
  ~StartupNotifyWatcher();

 signals:
  void appLaunched(const QString& name, uint userid, int rootpid);

 private:
  xcb_connection_t* xconn = nullptr;
  xcb_atom_t xatom_begin;
  xcb_atom_t xatom_info;
  xcb_atom_t xatom_id;
  xcb_atom_t xatom_utf8;

  QByteArray m_buffer;
  QMap<QString, QString> m_values;
  QSocketNotifier* m_notifier;
  bool m_corrupt;

  static xcb_atom_t xatomLookup(xcb_connection_t*, const char* name);
  void xeventStartupData(const xcb_client_message_event_t* msg);

 private slots:
  void xeventReady();
};

#endif  // STARTUPNOTIFYWATCHER_H
