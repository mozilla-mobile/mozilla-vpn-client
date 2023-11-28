/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LOCALSOCKETCONTROLLER_H
#define LOCALSOCKETCONTROLLER_H

#include <QLocalSocket>
#include <QTimer>
#include <functional>

#include "controllerimpl.h"

class QJsonObject;

class LocalSocketController final : public ControllerImpl {
  Q_DISABLE_COPY_MOVE(LocalSocketController)

 public:
  LocalSocketController();
  ~LocalSocketController();

  void initialize(const Device* device, const Keys* keys) override;

  void activate(const InterfaceConfig& config,
                Controller::Reason Reason) override;

  void deactivate(Controller::Reason reason) override;

  void checkStatus() override;

  void getBackendLogs(std::function<void(const QString&)>&& callback) override;

  void cleanupBackendLogs() override;

  bool multihopSupported() override { return true; }

  void testDaemonCrash() override;

 private:
  // For messages that are expected to generate a synchronous response, this
  // defines the default time that we will wait before assuming an error in
  // the daemon has occured.
  static constexpr int CONNECTION_RESPONSE_TIMEOUT_MSEC = 500;

  void initializeInternal();
  void disconnectInternal();

  void daemonConnected();
  void errorOccurred(QLocalSocket::LocalSocketError socketError);
  void readData();
  void parseCommand(const QByteArray& command);
  void clearTimeout(const QString& responseType);
  void clearAllTimeouts();

  /**
   * @brief Write a JSON message to the socket, sending it to the daemon.
   *
   * @param message - A JSON object describing the message to be sent.
   * @param expectedResponseType - An optional message type that we expect as
   *                               a response.
   * @param timeout - The timeout, in milliseconds, to wait for the response.
   * @return * void
   */
  void write(const QJsonObject& message,
             const QString& expectedResponseType = QString(),
             int timeout = CONNECTION_RESPONSE_TIMEOUT_MSEC);

 private:
  enum {
    eUnknown,
    eInitializing,
    eReady,
    eDisconnected,
  } m_daemonState = eUnknown;

  QLocalSocket* m_socket = nullptr;

  QByteArray m_buffer;

  std::function<void(const QString&)> m_logCallback = nullptr;

  QTimer m_initializingTimer;
  uint32_t m_initializingInterval = 0;

  // When a message to the daemon expects an immediate response, these
  // are used to trigger a timeout error if the response never arrives.
  QList<QTimer*> m_expectedResponses;
};

#endif  // LOCALSOCKETCONTROLLER_H
