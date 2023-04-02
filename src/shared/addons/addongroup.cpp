/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "addongroup.h"

#include "leakdetector.h"
#include "logger.h"

namespace {
Logger logger("AddonGroup");
}  // namespace

AddonGroup::AddonGroup(QObject* parent, const QList<Addon*>& addons,
                       const QString& manifestFileName, const QString& id,
                       const QString& name)
    : Addon(parent, manifestFileName, id, name), m_addons(addons) {
  MZ_COUNT_CTOR(AddonGroup);

  for (Addon* addon : m_addons) {
    addon->setParent(this);

    connect(addon, &Addon::retranslationCompleted, this, [this]() {
      if (m_retranslationCompletedCount > 0 &&
          --m_retranslationCompletedCount == 0) {
        emit retranslationCompleted();
      }
    });
  }
}

AddonGroup::~AddonGroup() { MZ_COUNT_DTOR(AddonGroup); }

void AddonGroup::enable() {
  for (Addon* addon : m_addons) {
    addon->enable();
  }

  Addon::enable();
}

void AddonGroup::disable() {
  for (Addon* addon : m_addons) {
    if (addon->enabled()) {
      addon->disable();
    }
  }

  Addon::disable();
}

void AddonGroup::enable(Type type) {
  Addon* addon = asInternal(type);
  if (addon && !addon->enabled()) {
    addon->enable();
    emit dataChanged();
  }
}

void AddonGroup::disable(Type type) {
  Addon* addon = asInternal(type);
  if (addon && addon->enabled()) {
    addon->disable();
    emit dataChanged();
  }
}

void AddonGroup::retranslate() {
  m_retranslationCompletedCount = m_addons.length();

  Addon::retranslate();

  for (Addon* addon : m_addons) {
    addon->retranslate();
  }
}

Addon* AddonGroup::asInternal(Type type) {
  for (Addon* addon : m_addons) {
    addon = addon->as(type);
    if (addon) {
      return addon;
    }
  }
  return nullptr;
}

Addon* AddonGroup::as(Type type) {
  Addon* addon = asInternal(type);
  if (addon && addon->enabled()) {
    return addon;
  }
  return nullptr;
}

void AddonGroup::setState(AddonState* state) {
  for (Addon* addon : m_addons) {
    addon->setState(state);
  }
  Addon::setState(state);
}
