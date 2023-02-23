/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "sessionstate.h"

#include "leakdetector.h"

SessionState::SessionState(const QJsonObject& initialState)
    : StateBase(initialState) {
  MZ_COUNT_CTOR(SessionState);
}

SessionState::~SessionState() { MZ_COUNT_DTOR(SessionState); }

QJsonValue SessionState::getInternal(const QString& key) const {
  return m_state[key];
}

void SessionState::setInternal(const QString& key, const QJsonValue& value) {
  m_state[key] = value;
}

void SessionState::clearInternal(const QString& key) {
  if (key.isEmpty()) {
    m_state.clear();
  }

  m_state.remove(key);
}
