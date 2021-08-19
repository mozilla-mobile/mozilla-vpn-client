/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "timercontroller.h"
#include "leakdetector.h"
#include "logger.h"

namespace {
Logger logger(LOG_CONTROLLER, "TimerController");
}

TimerController::TimerController(ControllerImpl* impl) : m_impl(impl) {
  MVPN_COUNT_CTOR(TimerController);

  Q_ASSERT(m_impl);
  m_impl->setParent(this);

  connect(m_impl, &ControllerImpl::initialized, this,
          &ControllerImpl::initialized);
  connect(m_impl, &ControllerImpl::connected,
          [this] { TimerController::maybeDone(true); });
  connect(m_impl, &ControllerImpl::disconnected,
          [this] { TimerController::maybeDone(false); });
  connect(m_impl, &ControllerImpl::statusUpdated, this,
          &ControllerImpl::statusUpdated);

  m_timer.setSingleShot(true);
  connect(&m_timer, &QTimer::timeout, this, &TimerController::timeout);
}

TimerController::~TimerController() { MVPN_COUNT_DTOR(TimerController); }

void TimerController::initialize(const Device* device, const Keys* keys) {
  m_impl->initialize(device, keys);
}

void TimerController::activate(
    const QList<Server>& serverList, const Device* device, const Keys* keys,
    const QList<IPAddressRange>& allowedIPAddressRanges,
    const QList<QString>& vpnDisabledApps, const QHostAddress& dns,
    Reason reason) {
  if (m_state != None) {
    return;
  }

  m_state = Connecting;

  if (reason != ReasonSwitching) {
    m_timer.stop();
    m_timer.start(TIME_ACTIVATION);
  }

  m_impl->activate(serverList, device, keys, allowedIPAddressRanges,
                   vpnDisabledApps, dns, reason);
}

void TimerController::deactivate(Reason reason) {
  if (m_state != None) {
    return;
  }

  m_state = Disconnecting;

  m_timer.stop();

  switch (reason) {
    case ReasonSwitching:
      m_timer.start(TIME_SWITCHING);
      break;
    case ReasonConfirming:
      m_timer.start(0);
      break;
    default:
      Q_ASSERT(reason == ReasonNone);
      m_timer.start(TIME_DEACTIVATION);
  }

  m_impl->deactivate(reason);
}

void TimerController::timeout() {
  logger.warning() << "TimerController - Timeout:" << m_state;

  Q_ASSERT(m_state != None);

  if (m_state == Connected) {
    m_state = None;
    emit connected();
    return;
  }

  if (m_state == Disconnected) {
    m_state = None;
    emit disconnected();
    return;
  }

  // Any other state can be ignored.
}

void TimerController::maybeDone(bool isConnected) {
  logger.debug() << "TimerController - Operation completed:" << m_state
                 << isConnected;

  if (m_state == Connecting) {
    if (m_timer.isActive()) {
      // The connection was faster.
      m_state = isConnected ? Connected : Disconnected;
      return;
    }

    // The timer was faster.
    m_state = None;

    if (isConnected) {
      emit connected();
    } else {
      emit disconnected();
    }
    return;
  }

  if (m_state == Disconnecting) {
    if (m_timer.isActive()) {
      // The disconnection was faster.
      m_state = Disconnected;
      return;
    }

    // The timer was faster.
    m_state = None;
    emit disconnected();
    return;
  }

  // External events could trigger the following codes.

  Q_ASSERT(m_state == None);
  Q_ASSERT(!m_timer.isActive());

  if (isConnected) {
    emit connected();
    return;
  }

  emit disconnected();
}

void TimerController::checkStatus() { m_impl->checkStatus(); }

void TimerController::getBackendLogs(
    std::function<void(const QString&)>&& a_callback) {
  std::function<void(const QString&)> callback = std::move(a_callback);
  m_impl->getBackendLogs(std::move(callback));
}

void TimerController::cleanupBackendLogs() { m_impl->cleanupBackendLogs(); }
