/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef XDGPORTAL_H
#define XDGPORTAL_H

#include <QDBusInterface>
#include <QObject>

constexpr const char* XDG_PORTAL_SERVICE = "org.freedesktop.portal.Desktop";
constexpr const char* XDG_PORTAL_PATH = "/org/freedesktop/portal/desktop";

constexpr const char* XDG_PORTAL_BACKGROUND =
    "org.freedesktop.portal.Background";
constexpr const char* XDG_PORTAL_REQUEST = "org.freedesktop.portal.Request";
constexpr const char* XDG_PORTAL_SECRET = "org.freedesktop.portal.Secret";
constexpr const char* XDG_PORTAL_SETTINGS = "org.freedesktop.portal.Settings";

// XDG Portals allow sandboxed applications to interact with the host operating
// system in a secure way via D-Bus APIs. This class implements some common
// conventions in the portal APIs to support things like sessions, asynchronous
// requests and determining the GUI window indentifiers.
//
// See https://flatpak.github.io/xdg-desktop-portal/docs/ for more information.
//
class XdgPortal : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(XdgPortal)

 public:
  explicit XdgPortal(const QString& interface, QObject* parent = nullptr);
  ~XdgPortal();

  const QString& token() const { return m_token; }
  const QString& replyPath() const { return m_replyPath; }
  void setReplyPath(const QString& path);
  uint getVersion();
  static void setupAppScope(const QString& appid);

 signals:
  void xdgResponse(uint, QVariantMap);

 private slots:
  void handleDbusResponse(uint, QVariantMap);

 protected:
  QString parentWindow();

  QString m_replyPath;
  QString m_token;

  QDBusInterface m_portal;
};

#endif  // XDGPORTAL_H
