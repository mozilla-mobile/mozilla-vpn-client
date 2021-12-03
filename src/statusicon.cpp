/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "statusicon.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"

#include <array>

namespace {
Logger logger(LOG_MAIN, "StatusIcon");

#if defined(MVPN_LINUX) || defined(MVPN_WINDOWS)
constexpr const std::array<const char*, 4> ANIMATED_LOGO_STEPS = {
    ":/ui/resources/logo-animated1.png", ":/ui/resources/logo-animated2.png",
    ":/ui/resources/logo-animated3.png", ":/ui/resources/logo-animated4.png"};

constexpr const char* LOGO_ON = ":/ui/resources/logo-on.png";
constexpr const char* LOGO_GENERIC = ":/ui/resources/logo-generic.png";
#else
constexpr const std::array<const char*, 4> ANIMATED_LOGO_STEPS = {
    ":/ui/resources/logo-animated1.svg", ":/ui/resources/logo-animated2.svg",
    ":/ui/resources/logo-animated3.svg", ":/ui/resources/logo-animated4.svg"};

constexpr const char* LOGO_ON = ":/ui/resources/logo-on.svg";
constexpr const char* LOGO_GENERIC = ":/ui/resources/logo-generic.svg";
#endif

}  // namespace

StatusIcon::StatusIcon() : m_icon(LOGO_GENERIC) {
  MVPN_COUNT_CTOR(StatusIcon);

  connect(&m_animatedIconTimer, &QTimer::timeout, this,
          &StatusIcon::animateIcon);
}

StatusIcon::~StatusIcon() { MVPN_COUNT_DTOR(StatusIcon); }

void StatusIcon::activateAnimation() {
  m_animatedIconIndex = 0;
  m_animatedIconTimer.start(Constants::statusIconAnimationMsec());
  animateIcon();
}

void StatusIcon::animateIcon() {
  Q_ASSERT(m_animatedIconIndex < ANIMATED_LOGO_STEPS.size());
  setIcon(ANIMATED_LOGO_STEPS[m_animatedIconIndex++]);
  if (m_animatedIconIndex == ANIMATED_LOGO_STEPS.size()) {
    m_animatedIconIndex = 0;
  }
}

void StatusIcon::stateChanged() {
  logger.debug() << "Show notification";

  m_animatedIconTimer.stop();

  MozillaVPN* vpn = MozillaVPN::instance();

  // If we are in a non-main state, we don't need to show special icons.
  if (vpn->state() != MozillaVPN::StateMain) {
    setIcon(LOGO_GENERIC);
    return;
  }

  switch (vpn->controller()->state()) {
    case Controller::StateOn:
      setIcon(LOGO_ON);
      break;

    case Controller::StateOff:
      setIcon(LOGO_GENERIC);
      break;

    case Controller::StateSwitching:
      [[fallthrough]];
    case Controller::StateConnecting:
      [[fallthrough]];
    case Controller::StateConfirming:
      [[fallthrough]];
    case Controller::StateDisconnecting:
      activateAnimation();
      break;

    default:
      setIcon(LOGO_GENERIC);
      break;
  }
}

void StatusIcon::setIcon(const QString& icon) {
  m_icon = icon;
  emit iconChanged(icon);
}

QUrl StatusIcon::iconUrl() const { return QUrl(QString("qrc%1").arg(m_icon)); }
