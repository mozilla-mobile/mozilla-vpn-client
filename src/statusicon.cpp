/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "statusicon.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"

#include <array>
#include <QBitmap>
#include <QFileInfo>
#include <QPainter>
#include <QPixmap>

namespace {
Logger logger(LOG_MAIN, "StatusIcon");

constexpr const QColor GREEN_COLOR = QColor(63, 225, 176, 255);
constexpr const QColor ORANGE_COLOR = QColor(255, 164, 54, 255);
constexpr const QColor RED_COLOR = QColor(226, 40, 80, 255);

#if defined(MVPN_LINUX) || defined(MVPN_WINDOWS)
constexpr const std::array<const char*, 4> ANIMATED_LOGO_STEPS = {
    ":/ui/resources/logo-animated1.png", ":/ui/resources/logo-animated2.png",
    ":/ui/resources/logo-animated3.png", ":/ui/resources/logo-animated4.png"};

constexpr const char* LOGO_GENERIC = ":/ui/resources/logo-generic.png";
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

StatusIcon::StatusIcon() : m_iconUrl(LOGO_GENERIC) {
  MVPN_COUNT_CTOR(StatusIcon);

  connect(&m_animatedIconTimer, &QTimer::timeout, this,
          &StatusIcon::animateIcon);

  // On Linux and Windows we’ll have to redraw the status indicator onto
  // the systray icon resource.
#if defined(MVPN_LINUX) || defined(MVPN_WINDOWS)
  connect(this, &StatusIcon::indicatorColorChanged,
          [this] { setIcon(m_iconUrl, false); });
#endif
}

StatusIcon::~StatusIcon() { MVPN_COUNT_DTOR(StatusIcon); }

void StatusIcon::activateAnimation() {
  m_animatedIconIndex = 0;
  m_animatedIconTimer.start(Constants::statusIconAnimationMsec());
  animateIcon();
}

void StatusIcon::animateIcon() {
  Q_ASSERT(m_animatedIconIndex < ANIMATED_LOGO_STEPS.size());
  setIcon(ANIMATED_LOGO_STEPS[m_animatedIconIndex++], false);
  if (m_animatedIconIndex == ANIMATED_LOGO_STEPS.size()) {
    m_animatedIconIndex = 0;
  }
}

void StatusIcon::stabilityChanged() {
  logger.debug() << "Stability changed";

  switch (MozillaVPN::instance()->connectionHealth()->stability()) {
    case ConnectionHealth::Stable:
      setIndicatorColor(GREEN_COLOR);
      break;
    case ConnectionHealth::Unstable:
      setIndicatorColor(ORANGE_COLOR);
      break;
    case ConnectionHealth::NoSignal:
      setIndicatorColor(RED_COLOR);
      break;
    default:
      logger.error() << "Unhandled status indicator for connection stability";
      setIndicatorColor(QColor());
  }
}

const QIcon& StatusIcon::icon() {
  if (m_icon.isNull()) {
    stateChanged();
    Q_ASSERT(!m_icon.isNull());
  }

  return m_icon;
}

void StatusIcon::stateChanged() {
  logger.debug() << "State changed";

  m_animatedIconTimer.stop();

  MozillaVPN* vpn = MozillaVPN::instance();
  Q_ASSERT(vpn);

  // If we are in a non-main state, we don't need to show special icons.
  if (vpn->state() != MozillaVPN::StateMain) {
    setIcon(LOGO_GENERIC, false);
    setIndicatorColor(QColor());
    return;
  }

  switch (vpn->controller()->state()) {
#if defined(MVPN_LINUX) || defined(MVPN_WINDOWS)
    case Controller::StateOn:
      [[fallthrough]];
    case Controller::StateOff:
      setIcon(LOGO_GENERIC, true);
      break;
#else
    case Controller::StateOn:
      setIcon(LOGO_GENERIC_ON, false);
      // Make sure the indicator color is set initially
      setIndicatorColor(GREEN_COLOR);
      break;
    case Controller::StateOff:
      setIcon(LOGO_GENERIC_OFF, false);
      setIndicatorColor(QColor());
      break;
#endif
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
      setIcon(LOGO_GENERIC, false);
      break;
  }
}

QIcon StatusIcon::drawStatusIndicator(const QString& iconUrl) const {
  logger.debug() << "Get icon from URL";

  // Create pixmap so that we can paint on the original resource.
  QPixmap iconPixmap = QPixmap(iconUrl);
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
  painter.setBrush(m_indicatorColor);
  painter.drawEllipse(indicatorDot);

  return QIcon(iconPixmap);
}

void StatusIcon::setIndicatorColor(const QColor& color) {
  logger.debug() << "Set indicator color";

  m_indicatorColor = color;
  emit indicatorColorChanged();
}

void StatusIcon::setIcon(const QString& iconUrl, bool shouldDrawIndicator) {
  logger.debug() << "Set icon" << iconUrl;

  m_iconUrl = iconUrl;

  // If we are in state main and the VPN is on we draw the status indicator
  // directly onto the image Windows and Linux.
  if (shouldDrawIndicator) {
    m_icon = drawStatusIndicator(m_iconUrl);
  } else {
    m_icon = QIcon(m_iconUrl);
  }

  emit iconChanged();
}

QUrl StatusIcon::iconUrl() const {
  return QUrl(QString("qrc%1").arg(m_iconUrl));
}
