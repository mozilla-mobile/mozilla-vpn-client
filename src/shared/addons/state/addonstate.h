/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADDONSTATE_H
#define ADDONSTATE_H

#include <QJsonObject>
#include <QObject>

#include "addonsessionstate.h"

/**
 * @brief Struct containing the different types of state an addon can hold.
 *
 * These are:
 *
 * - `session` state: A type of state with is not persisted throughout device
 * sessions.
 *
 */
struct AddonState : public QObject {
  Q_OBJECT
  Q_PROPERTY(AddonSessionState* session READ session)

 public:
  /**
   * @brief Construct a new Addon State object.
   *
   * No validation is required here.
   *
   * 1. The session object construction will validate the object.
   * 2. In case it is empty, an empty state will be generated. Empty states
   * are allowed so that the state APIs will always be available. Attempts
   * to record to invalid keys will simply be no-ops.
   *
   * @param manifest The JSON object in the "state" property of an addon
   * manifest.
   */
  AddonState(QObject* parent, const QJsonObject& manifest)
      : QObject(parent), m_session(manifest["session"].toObject()) {}
  ~AddonState() = default;

  AddonSessionState* session() { return &m_session; }

 private:
  AddonSessionState m_session;
};

#endif  // ADDONSTATE_H
