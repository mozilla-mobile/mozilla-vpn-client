/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "navigationbarbutton.h"

#include <QCoreApplication>
#include <QFile>

#include "leakdetector.h"
#include "logger.h"

namespace {
Logger logger("NavigationBarButton");
}

NavigationBarButton::NavigationBarButton(
    QObject* parent, const QString& objectName,
    const QString& navAccessibleName, int screen,
    const QString& sourceUnchecked, const QString& sourceChecked,
    const QString& sourceUncheckedNotification,
    const QString& sourceCheckedNotification)
    : QObject(parent),
      m_objectName(objectName),
      m_navAccessibleName(navAccessibleName),
      m_sourceUnchecked(sourceUnchecked),
      m_sourceChecked(sourceChecked),
      m_sourceUncheckedNotification(sourceUncheckedNotification),
      m_sourceCheckedNotification(sourceCheckedNotification),
      m_screen(screen) {
  MZ_COUNT_CTOR(NavigationBarButton);
}

NavigationBarButton::~NavigationBarButton() {
  MZ_COUNT_DTOR(NavigationBarButton);
}

const QString& NavigationBarButton::source() const {
  if (m_checked) {
    return m_hasNotification && !m_sourceCheckedNotification.isEmpty()
               ? m_sourceCheckedNotification
               : m_sourceChecked;
  }

  return m_hasNotification ? m_sourceUncheckedNotification : m_sourceUnchecked;
}

void NavigationBarButton::setChecked(bool checked) {
  if (m_checked != checked) {
    m_checked = checked;
    emit sourceChanged();
  }
}

void NavigationBarButton::setHasNotification(bool hasNotification) {
  if (m_hasNotification != hasNotification) {
    m_hasNotification = hasNotification;
    emit sourceChanged();
  }
}
