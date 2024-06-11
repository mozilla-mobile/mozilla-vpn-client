/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "ipaddresslookup.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include "controller.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "tasks/ipfinder/taskipfinder.h"
#include "taskscheduler.h"

namespace {
Logger logger("IpAddressLookup");
}

IpAddressLookup::IpAddressLookup() {
  MZ_COUNT_CTOR(IpAddressLookup);

  reset();

  connect(&m_ipAddressTimer, &QTimer::timeout, this,
          [this]() { updateIpAddress(); });
}

IpAddressLookup::~IpAddressLookup() { MZ_COUNT_DTOR(IpAddressLookup); }

void IpAddressLookup::initialize() {
  MozillaVPN* vpn = MozillaVPN::instance();

  connect(vpn, &MozillaVPN::stateChanged, this, &IpAddressLookup::stateChanged);

  connect(vpn->controller(), &Controller::stateChanged, this,
          &IpAddressLookup::stateChanged);
}

void IpAddressLookup::reset() {
  logger.debug() << "Resetting the data";

  if (m_state != StateWaiting) {
    //% "Loading"
    //: This refers to the current IP address, i.e. "IP: Loading".
    m_ipv4Address = qtTrId("vpn.connectionInfo.loading");
    m_ipv6Address = qtTrId("vpn.connectionInfo.loading");

    m_state = StateWaiting;
  }
}

void IpAddressLookup::updateIpAddress() {
  logger.debug() << "Updating IP address";

  if (m_state == StateUpdating) {
    return;
  }
  m_state = StateUpdating;

  auto* ipfinder = new TaskIPFinder();
  connect(
      ipfinder, &TaskIPFinder::operationCompleted, this,
      [this](const QString& ipv4, const QString& ipv6, const QString& country) {
        if (ipv4.isEmpty() && ipv6.isEmpty()) {
          logger.error() << "IP address request failed";
          m_state = StateUpdated;
          emit ipAddressChecked();
          return;
        }

        logger.debug() << "IP address request completed";

    // Let's skip this for unit-tests to make them simpler.
#ifndef UNIT_TEST
        // The country check should follow the country in use by the controller
        // and not the MozillaVPN::serverData() which could be changed in the
        // meantime, because of a server-switch request.
        if (country != MozillaVPN::instance()
                           ->controller()
                           ->currentServer()
                           .exitCountryCode()) {
          // In case the country-we're reported in does not match the
          // connected server we may retry only once.
          logger.warning() << "Reported ip not in the right country, retry!";
          QTimer::singleShot(3000, this, [this]() {
            // Maybe we have nothing to do now because the VPN is off or the
            // user is not authenticated anymore.
            MozillaVPN* vpn = MozillaVPN::instance();
            if (vpn->state() == App::StateMain &&
                vpn->controller()->state() == Controller::StateOn) {
              updateIpAddress();
            }
          });
        }
#endif

        if (!ipv4.isEmpty()) {
          m_ipv4Address = ipv4;
          emit ipv4AddressChanged();
        }

        if (!ipv4.isEmpty()) {
          m_ipv6Address = ipv6;
          emit ipv6AddressChanged();
        }

        logger.debug() << "Set own Address. ipv4:"
                       << logger.sensitive(m_ipv4Address)
                       << "ipv6:" << logger.sensitive(m_ipv6Address) << "in"
                       << logger.sensitive(country);

        m_state = StateUpdated;
        emit ipAddressChecked();
      });

  TaskScheduler::scheduleTask(ipfinder);
}

void IpAddressLookup::stateChanged() {
  logger.debug() << "state changed";

  MozillaVPN* vpn = MozillaVPN::instance();

  if (vpn->state() != App::StateMain ||
      vpn->controller()->state() != Controller::StateOn) {
    reset();
    return;
  }

  updateIpAddress();
}
