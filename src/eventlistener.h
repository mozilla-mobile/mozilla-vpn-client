/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef EVENTLISTENER_H
#define EVENTLISTENER_H

#include <QLocalServer>
#include <QObject>

class EventListener final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(EventListener)

 public:
  EventListener();
  ~EventListener();

  /**
   * @brief Check for the existence of other instances of the program.
   *
   * @param windowTitle - expected name of the UI window.
   * @return * true if another instance was found.
   */
  static bool checkForInstances(const QString& windowTitle);

  /**
   * @brief Send a deep link command to the other instance of the program.
   *
   * @param url - deep link URL
   * @return * true if the command was sent successfully.
   */
  static bool sendDeepLink(const QUrl& url);

 private:
  void socketReadyRead();
  void handleLinkCommand(const QString& payload);

  static bool sendCommand(const QString& data);

 private:
  QLocalServer m_server;
};

#endif  // EVENTLISTENER_H
