
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <QObject>
#include <QPointer>
#include <QProcess>
#include <QProperty>
#include <QUrl>

class ProxyController {
 public:
  ProxyController() { m_state = Stopped{}; };
  ~ProxyController() = default;

  void start();
  void stop();

  struct Stopped {};
  struct Started {
    // a socks:// url that can be used to connect to the Proxy.
    QUrl url;
  };
  using State = std::variant<Started, Stopped>;

  // Returns true if the proxy is alive
  State state() { return m_state.value(); }
  // Returns a bindable in case state changes.
  QBindable<State> stateBindable() { return QBindable<State>(&m_state); }

  /**
   * @brief Returns the Path of the Binary to Exclude
   *
   * @return const QString - Path :)
   */
  static const QString binaryPath();
  /**
   * Returns true if an activation is possible
   */
  bool canActivate();

  /**
   * @brief Generates a pseudo-random socks5URL
   *
   * @return QURL
   */
  static QUrl generateProxyUrl();

  /**
   * @brief Get the Arguments for the socksproxy process
   *
   * @param proxyURL - The expected socks5 Proxy url
   * @return const QStringList - arguments to pass to socksproxy to achive that
   * state.
   */
  static const QStringList getArguments(const QUrl& proxyURL);

 private:
  QProperty<State> m_state;
  QPointer<QProcess> mCurrentProcess;
  QMetaObject::Connection mCrashSignal;
  std::optional<bool> m_canActivate;
};
