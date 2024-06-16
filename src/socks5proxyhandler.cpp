/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "socks5proxyhandler.h"

#include <QCoreApplication>

#include "../socks5proxy/src/socks5.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/feature.h"
#include "mozillavpn.h"
#include "settingsholder.h"

namespace {
Logger logger(LOG_MAIN, "Socks5ProxyHandler");

Socks5ProxyHandler* s_instance = nullptr;
}  // namespace

// static
Socks5ProxyHandler* Socks5ProxyHandler::instance() {
  if (!s_instance) {
    s_instance = new Socks5ProxyHandler(qApp);
  }

  return s_instance;
}

Socks5ProxyHandler::Socks5ProxyHandler(QObject* parent) : QObject(parent) {
  MVPN_COUNT_CTOR(Socks5ProxyHandler);

  MozillaVPN* vpn = MozillaVPN::instance();
  Q_ASSERT(vpn);

  connect(vpn->controller(), &Controller::stateChanged, this,
          &Socks5ProxyHandler::stateChanged);

  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  connect(settingsHolder, &SettingsHolder::socks5ProxyEnabledChanged, this,
          &Socks5ProxyHandler::stateChanged);
  connect(settingsHolder, &SettingsHolder::socks5ProxyPortChanged, this,
          &Socks5ProxyHandler::stateChanged);

  stateChanged();
}

Socks5ProxyHandler::~Socks5ProxyHandler() {
  MVPN_COUNT_DTOR(Socks5ProxyHandler);
}

void Socks5ProxyHandler::stateChanged() {
  if (!Feature::get(Feature::Feature_socks5proxy)->isSupported()) {
    return;
  }

  if (!SettingsHolder::instance()->socks5ProxyEnabled()) {
    maybeTurnOff();
    return;
  }

  MozillaVPN* vpn = MozillaVPN::instance();
  Q_ASSERT(vpn);

  switch (vpn->controller()->state()) {
    case Controller::StateOff:
      maybeTurnOff();
      break;
    case Controller::StateOn:
      maybeTurnOn();
      break;
    default:
      break;
  }
}

void Socks5ProxyHandler::maybeTurnOff() {
  logger.debug() << "Maybe turning off the proxy";
  if (m_socks5Proxy) {
    m_socks5Proxy->deleteLater();
    m_socks5Proxy = nullptr;
    emit connectionsChanged();
  }
}

void Socks5ProxyHandler::maybeTurnOn() {
  logger.debug() << "Maybe turning on the proxy";

  int port = SettingsHolder::instance()->socks5ProxyPort();

  if (m_socks5Proxy && m_socks5Proxy->port() != port) {
    m_socks5Proxy->deleteLater();
    m_socks5Proxy = nullptr;
  }

  if (!m_socks5Proxy) {
    m_socks5Proxy = new Socks5(this, port);

    connect(m_socks5Proxy, &Socks5::connectionsChanged, this,
            &Socks5ProxyHandler::connectionsChanged);

    emit connectionsChanged();
  }
}

uint16_t Socks5ProxyHandler::connections() const {
  if (!m_socks5Proxy) {
    return 0;
  }
  return m_socks5Proxy->connections();
}
