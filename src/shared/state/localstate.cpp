/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "localstate.h"

#include <QSettings>

#include "leakdetector.h"
#include "logger.h"
#include "settingsholder.h"

namespace {
Logger logger("LocalState");

// Stores a list of identifiers for LocalState instances created during this
// application run. Creating two that point to the same storage space is
// likely a bug and will generated confusion from two places of the
// code writing and reading from the same place in storage.
QList<QString> s_registeredStorageIdentifiers;
// The identifier of the settings group that will contain
// ALL of the persisted state.
const char* s_localStateStorageIdentifier = "mzLocalState";
}  // namespace

// static
QString LocalState::getStorageIdentifierForKey(const QString& storageIdentifier,
                                               const QString& key) {
  return QString("%1/%2/%3")
      .arg(s_localStateStorageIdentifier, storageIdentifier, key);
}

LocalState::LocalState(const QString& storageIdentifier,
                       const QJsonObject& initialState)
    : StateBase(initialState), m_storageIdentifier(storageIdentifier) {
  if (s_registeredStorageIdentifiers.contains(storageIdentifier)) {
    logger.warning() << "Creating a new LocalState instance with the same "
                        "storageIdentifier as an existing LocalState "
                        "instance. That is likely a bad idea.";
  } else {
    s_registeredStorageIdentifiers.append(storageIdentifier);
  }

  MZ_COUNT_CTOR(LocalState);
}

LocalState::~LocalState() { MZ_COUNT_DTOR(LocalState); }

QJsonValue LocalState::getInternal(const QString& key) const {
  return settings()
      ->value(LocalState::getStorageIdentifierForKey(m_storageIdentifier, key))
      .toJsonValue();
}

void LocalState::setInternal(const QString& key, const QJsonValue& value) {
  settings()->setValue(
      LocalState::getStorageIdentifierForKey(m_storageIdentifier, key), value);
}

void LocalState::clearInternal(const QString& key) {
  settings()->beginGroup(s_localStateStorageIdentifier);

  if (key.isEmpty()) {
    settings()->remove(m_storageIdentifier);
  } else {
    settings()->remove(key);
  }

  settings()->endGroup();
}
