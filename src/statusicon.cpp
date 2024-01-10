/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "statusicon.h"

#include <QBitmap>
#include <QFileInfo>
#include <QPainter>
#include <QPixmap>
#include <array>

#include "context/constants.h"
#include "controller.h"
#include "leakdetector.h"
#include "logging/logger.h"
#include "mozillavpn.h"

namespace {
Logger logger("StatusIcon");

constexpr const QColor GREEN_COLOR = QColor(63, 225, 176, 255);
constexpr const QColor ORANGE_COLOR = QColor(255, 164, 54, 255);
constexpr const QColor RED_COLOR = QColor(226, 40, 80, 255);
constexpr const QColor INVALID_COLOR = QColor();

#if defined(MZ_LINUX) || defined(MZ_WINDOWS)
constexpr const std::array<const char*, 4> ANIMATED_LOGO_STEPS = {
    ":/ui/resources/logo-animated1.png", ":/ui/resources/logo-animated2.png",
    ":/ui/resources/logo-animated3.png", ":/ui/resources/logo-animated4.png"};

constexpr const char* LOGO_GENERIC = ":/ui/resources/logo-generic.png";
constexpr const char* LOGO_GENERIC_OFF = ":/ui/resources/logo-generic-off.png";
constexpr const char* LOGO_GENERIC_ON = LOGO_GENERIC;
#else
constexpr const std::array<const char*, 4> ANIMATED_LOGO_STEPS = {
    ":/ui/resources/logo-animated-mask1.png",
    ":/ui/resources/logo-animated-mask2.png",
    ":/ui/resources/logo-animated-mask3.png",
    ":/ui/resources/logo-animated-mask4.png"};

constexpr const char* LOGO_GENERIC = ":/ui/resources/logo-generic-mask.png";
constexpr const char* LOGO_GENERIC_OFF =
    ":/ui/resources/logo-generic-mask-off.png";
constexpr const char* LOGO_GENERIC_ON =
    ":/ui/resources/logo-generic-mask-on.png";
#endif

}  // namespace

StatusIcon::StatusIcon() {
  MZ_COUNT_CTOR(StatusIcon);

  connect(&m_animatedIconTimer, &QTimer::timeout, this,
          &StatusIcon::animateIcon);
}

StatusIcon::~StatusIcon() { MZ_COUNT_DTOR(StatusIcon); }

const QIcon& StatusIcon::icon() {
  if (m_icon.isNull()) {
    m_icon = drawStatusIndicator();
    Q_ASSERT(!m_icon.isNull());
  }

  return m_icon;
}

void StatusIcon::activateAnimation() {
  logger.debug() << "Activate animation";
  m_animatedIconIndex = 0;
  m_animatedIconTimer.start(Constants::statusIconAnimationMsec());
}

void StatusIcon::animateIcon() {
  logger.debug() << "Animate icon";

  Q_ASSERT(m_animatedIconIndex < ANIMATED_LOGO_STEPS.size());
  m_animatedIconIndex++;
  if (m_animatedIconIndex == ANIMATED_LOGO_STEPS.size()) {
    m_animatedIconIndex = 0;
  }
  refreshNeeded();
}

const QString StatusIcon::iconString() {
  logger.debug() << "Icon string" << m_animatedIconIndex;

  MozillaVPN* vpn = MozillaVPN::instance();

  // If we are in a non-main state, we don't need to show special icons.
  if (vpn->state() != App::StateMain) {
    return LOGO_GENERIC;
  }

  switch (vpn->controller()->state()) {
    case Controller::StateOn:
      [[fallthrough]];
    case Controller::StateSilentSwitching:
      m_animatedIconTimer.stop();
      return LOGO_GENERIC_ON;
      break;
    case Controller::StateOff:
      m_animatedIconTimer.stop();
      return LOGO_GENERIC_OFF;
      break;
    case Controller::StateSwitching:
      [[fallthrough]];
    case Controller::StateConnecting:
      [[fallthrough]];
    case Controller::StateCheckSubscription:
      [[fallthrough]];
    case Controller::StateConfirming:
      [[fallthrough]];
    case Controller::StateDisconnecting:
      if (!m_animatedIconTimer.isActive()) {
        activateAnimation();
      }
      return ANIMATED_LOGO_STEPS[m_animatedIconIndex];
      break;
    default:
      m_animatedIconTimer.stop();
      return LOGO_GENERIC;
      break;
  }
}

const QColor StatusIcon::indicatorColor() const {
  logger.debug() << "Set color";

  MozillaVPN* vpn = MozillaVPN::instance();

  if (vpn->state() != App::StateMain ||
      vpn->controller()->state() != Controller::StateOn) {
    return INVALID_COLOR;
  }

  switch (vpn->connectionHealth()->stability()) {
    case ConnectionHealth::Stable:
      return GREEN_COLOR;
    case ConnectionHealth::Unstable:
      return ORANGE_COLOR;
    case ConnectionHealth::NoSignal:
      return RED_COLOR;
    default:
      logger.error() << "Unhandled status indicator for connection stability";
      return INVALID_COLOR;
  }
}

void StatusIcon::refreshNeeded() {
  logger.debug() << "Refresh needed";

  if (!m_icon.isNull()) {
    m_icon = QIcon();
  }
  emit iconUpdateNeeded();
}

QIcon StatusIcon::drawStatusIndicator() {
  logger.debug() << "Get icon from URL";

  // Create pixmap so that we can paint on the original resource.
  QPixmap iconPixmap = QPixmap(iconString());

  MozillaVPN* vpn = MozillaVPN::instance();

  // Only draw a status indicator if the VPN is connected
  if (vpn->controller()->state() == Controller::StateOn) {
    QPainter painter(&iconPixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    // Create mask for the indicator.
    float maskSize = iconPixmap.width() * 0.5;
    float maskPosition = iconPixmap.width() - maskSize;
    QRectF indicatorMask(maskPosition, maskPosition, maskSize, maskSize);
    painter.setBrush(QColor(0, 0, 0, 255));  // black
    painter.drawEllipse(indicatorMask);

    // Add a colored status indicator.
    float dotPadding = maskSize * 0.2;
    float dotSize = maskSize - dotPadding;
    float dotPosition = maskPosition + dotPadding * 0.5;
    QRectF indicatorDot(dotPosition, dotPosition, dotSize, dotSize);
    painter.setBrush(indicatorColor());
    painter.drawEllipse(indicatorDot);
  }

  return QIcon(iconPixmap);
}
