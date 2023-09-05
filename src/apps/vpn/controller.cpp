/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "controller.h"

#include "app.h"
#include "appconstants.h"
#include "apppermission.h"
#include "captiveportal/captiveportal.h"
#include "controllerimpl.h"
#include "dnshelper.h"
#include "feature.h"
#include "frontend/navigator.h"
#include "ipaddress.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/devicemodel.h"
#include "models/keys.h"
#include "models/server.h"
#include "models/servercountrymodel.h"
#include "mozillavpn.h"
#include "networkrequest.h"
#include "rfc/rfc1112.h"
#include "rfc/rfc1918.h"
#include "rfc/rfc4193.h"
#include "rfc/rfc4291.h"
#include "serveri18n.h"
#include "serverlatency.h"
#include "settingsholder.h"
#include "tasks/controlleraction/taskcontrolleraction.h"
#include "tasks/function/taskfunction.h"
#include "tasks/heartbeat/taskheartbeat.h"
#include "taskscheduler.h"
#include "tutorial/tutorial.h"

#if defined(MZ_LINUX)
#  include "platforms/linux/linuxcontroller.h"
#elif defined(MZ_MACOS) || defined(MZ_WINDOWS)
#  include "localsocketcontroller.h"
#elif defined(MZ_IOS)
#  include "platforms/ios/ioscontroller.h"
#elif defined(MZ_ANDROID)
#  include "platforms/android/androidcontroller.h"
#else
#  include "platforms/dummy/dummycontroller.h"
#endif

namespace {
Logger logger("Connection Manager");
}

Controller::Controller() { MZ_COUNT_CTOR(Controller); }

Controller::~Controller() { MZ_COUNT_DTOR(Controller); }

void Controller::initialize() {
  logger.debug() << "Initializing the controller";
  m_initialized = true;

  //  if (m_state != ConnectionManager::StateInitializing) {
  //    setState(ConnectionManager::StateInitializing);
  //  }

  // Let's delete the previous controller before creating a new one.
  if (m_impl) {
    m_impl.reset(nullptr);
  }

  //  m_serverData = *MozillaVPN::instance()->serverData();
  //  m_nextServerData = *MozillaVPN::instance()->serverData();

#if defined(MZ_LINUX)
  m_impl.reset(new LinuxController());
#elif defined(MZ_MACOS) || defined(MZ_WINDOWS)
  m_impl.reset(new LocalSocketController());
#elif defined(MZ_IOS)
  m_impl.reset(new IOSController());
#elif defined(MZ_ANDROID)
  m_impl.reset(new AndroidController());
#else
  m_impl.reset(new DummyController());
#endif

  //  connect(m_impl.get(), &ControllerImpl::connected, this,
  //          &ConnectionManager::connected);
  //  connect(m_impl.get(), &ControllerImpl::disconnected, this,
  //          &ConnectionManager::disconnected);
  //  connect(m_impl.get(), &ControllerImpl::initialized, this,
  //          &ConnectionManager::implInitialized);
  //  connect(m_impl.get(), &ControllerImpl::statusUpdated, this,
  //          &ConnectionManager::statusUpdated);
  //  connect(this, &ConnectionManager::stateChanged, this,
  //          &ConnectionManager::maybeEnableDisconnectInConfirming);

  //  connect(&m_pingCanary, &PingHelper::pingSentAndReceived, this, [this]() {
  //    m_pingCanary.stop();
  //    m_pingReceived = true;
  //    logger.info() << "Canary Ping Succeeded";
  //  });

  //  connect(SettingsHolder::instance(), &SettingsHolder::transactionBegan,
  //  this,
  //          [this]() {
  //            m_connectedBeforeTransaction =
  //                m_state == ConnectionManager::StateOn;
  //          });
  //
  //  connect(SettingsHolder::instance(),
  //          &SettingsHolder::transactionAboutToRollBack, this, [this]() {
  //            if (!m_connectedBeforeTransaction)
  //              MozillaVPN::instance()->deactivate();
  //          });
  //
  //  connect(SettingsHolder::instance(),
  //  &SettingsHolder::transactionRolledBack,
  //          this, [this]() {
  //            if (m_connectedBeforeTransaction)
  //              MozillaVPN::instance()->activate();
  //          });
  //
  //  MozillaVPN* vpn = MozillaVPN::instance();
  //
  //  const Device* device = vpn->deviceModel()->currentDevice(vpn->keys());
  //  m_impl->initialize(device, vpn->keys());
  //
  //  connect(SettingsHolder::instance(), &SettingsHolder::serverDataChanged,
  //  this,
  //          &ConnectionManager::serverDataChanged);
  //
  //  connect(LogHandler::instance(), &LogHandler::cleanupLogsNeeded, this,
  //          &ConnectionManager::cleanupBackendLogs);
  //
  //  connect(this, &ConnectionManager::readyToServerUnavailable,
  //          Tutorial::instance(), &Tutorial::stop);
}

bool Controller::isVPNActive() { return m_VPNActive; }

void Controller::serializeLogs(
    std::function<void(const QString& name, const QString& logs)>&&
        a_callback) {
  std::function<void(const QString& name, const QString&)> callback =
      std::move(a_callback);

  if (!m_impl) {
    callback("Mozilla VPN backend logs", QString());
    return;
  }

  m_impl->getBackendLogs([callback](const QString& logs) {
    callback("Mozilla VPN backend logs", logs);
  });
}

bool Controller::deactivate() {
  //  logger.debug() << "Deactivation" << m_state;
  logger.debug() << "Deactivation";

  //  if (m_state != StateOn && m_state != StateSwitching &&
  //      m_state != StateSilentSwitching && m_state != StateConfirming &&
  //      m_state != StateConnecting && m_state != StateCheckSubscription) {
  //    logger.warning() << "Already disconnected";
  //    return false;
  //  }
  //
  //  if (m_state == StateOn || m_state == StateConfirming ||
  //      m_state == StateConnecting || m_state == StateCheckSubscription) {
  //    setState(StateDisconnecting);
  //  }
  //
  //  m_pingCanary.stop();
  //  m_handshakeTimer.stop();
  //  m_activationQueue.clear();
  //  clearConnectedTime();
  //  clearRetryCounter();
  if (m_VPNActive) {
    m_VPNActive = false;
  } else {
    logger.warning() << "Already disconnected";
    return false;
  }

  Q_ASSERT(m_impl);
  // stop all ongoing checks
  //  m_impl->deactivate(stateToReason(m_state));
  return true;
}
