/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "statusicon.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"

#ifdef MVPN_MACOS
#  include "platforms/macos/macosutils.h"
#endif

#include <array>
#include <QBitmap>
#include <QPainter>
#include <QPixmap>

namespace {
Logger logger(LOG_MAIN, "StatusIcon");

#if defined(MVPN_LINUX) || defined(MVPN_WINDOWS)
constexpr const std::array<const char*, 4> ANIMATED_LOGO_STEPS = {
    ":/ui/resources/logo-animated1.png", ":/ui/resources/logo-animated2.png",
    ":/ui/resources/logo-animated3.png", ":/ui/resources/logo-animated4.png"};

constexpr const char* LOGO_GENERIC = ":/ui/resources/logo-generic.png";
#else
constexpr const std::array<const char*, 4> ANIMATED_LOGO_STEPS = {
    ":/ui/resources/logo-animated1.svg", ":/ui/resources/logo-animated2.svg",
    ":/ui/resources/logo-animated3.svg", ":/ui/resources/logo-animated4.svg"};

constexpr const char* LOGO_GENERIC = ":/ui/resources/logo-generic.svg";
#endif

}  // namespace

StatusIcon::StatusIcon() : m_iconUrl(LOGO_GENERIC) {
  MVPN_COUNT_CTOR(StatusIcon);

  MacOSUtils::test();
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

void StatusIcon::setEffectiveAppearance(bool isDarkAppearance) {
  logger.debug() << "Set effective appearance" << isDarkAppearance;

  if (isDarkAppearance) {
    m_effectiveAppearance = EffectiveAppearanceDark;
  } else {
    m_effectiveAppearance = EffectiveAppearanceLight;
  }
  setIcon(LOGO_GENERIC);

  emit effectiveAppearanceChanged();
}

void StatusIcon::stabilityChanged() {
  logger.debug() << "Stability changed";

  setIcon(LOGO_GENERIC);
}

void StatusIcon::stateChanged() {
  logger.debug() << "State changed";

  m_animatedIconTimer.stop();

  MozillaVPN* vpn = MozillaVPN::instance();
  Q_ASSERT(vpn);
  switch (vpn->controller()->state()) {
    case Controller::StateOn:
      [[fallthrough]];
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

QIcon StatusIcon::getIconFromUrl(const QString& iconUrl) const {
  logger.debug() << "Get icon from URL";

  MozillaVPN* vpn = MozillaVPN::instance();
  Q_ASSERT(vpn);
  // If we are in a non-main state, we don't need to show special icons.
  if (vpn->state() != MozillaVPN::StateMain) {
    return QIcon(iconUrl);
  }

  QColor color = QColor(63, 225, 176);  // green
  switch (MozillaVPN::instance()->connectionHealth()->stability()) {
    case ConnectionHealth::Stable:
      break;
    case ConnectionHealth::Unstable:
      color = QColor(255, 164, 54);  // orange
      break;
    case ConnectionHealth::NoSignal:
      color = QColor(226, 40, 80);  // red
    default:
      logger.error() << "Unhandled status indicator for connection stability";
      break;
  }

  // Create pixmap so that we can paint on the original resource
  QPixmap iconPixmap = QPixmap(iconUrl);
  QPainter painter(&iconPixmap);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setPen(Qt::NoPen);
  painter.setBrush(color);

  // Create mask for the indicator
  float maskSize = iconPixmap.width() * 0.5;
  float maskPosition = iconPixmap.width() - maskSize;
  QRectF indicatorMask(maskPosition, maskPosition, maskSize, maskSize);
  painter.setCompositionMode(QPainter::CompositionMode_Clear);
  painter.drawEllipse(indicatorMask);

  // Add a colored status indicator
  float dotPadding = maskSize * 0.2;
  float dotSize = maskSize - dotPadding;
  float dotPosition = maskPosition + dotPadding * 0.5;
  QRectF indicatorDot(dotPosition, dotPosition, dotSize, dotSize);
  painter.setCompositionMode(QPainter::CompositionMode_DestinationOver);
  painter.drawEllipse(indicatorDot);

  return QIcon(iconPixmap);
}

void StatusIcon::setIcon(const QString& iconUrl) {
  logger.debug() << "Set icon" << iconUrl;

  switch (m_effectiveAppearance) {
    case EffectiveAppearanceDark: {
      // Get the light version of the logo
      QStringList iconUrlParts = iconUrl.split(".");
      m_iconUrl = QString("%1-light.%2").arg(iconUrlParts[0])
                                        .arg(iconUrlParts[1]);
      break;
    }
    case EffectiveAppearanceLight: {
      [[fallthrough]];
    }
    default: {
      m_iconUrl = iconUrl;
      break;
    }
  }
  m_icon = getIconFromUrl(m_iconUrl);
  emit iconChanged(m_icon);
}

QUrl StatusIcon::iconUrl() const {
  return QUrl(QString("qrc%1").arg(m_iconUrl));
}
