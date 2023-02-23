/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef STATE_H
#define STATE_H

#include <QJsonObject>
#include <QObject>

#include "sessionstate.h"

/**
 * @brief Struct containing the different types of state an addon can hold.
 *
 * These are:
 *
 * - `session` state: A type of state with is not persisted throughout device
 * sessions.
 *
 */
struct State : public QObject {
  Q_OBJECT
  Q_PROPERTY(SessionState* session READ session)

 public:
  /**
   * @brief Construct a new State object.
   *
   * No validation is required here.
   *
   * 1. The session object construction will validate the object.
   * 2. In case it is empty, an empty state will be generated. Empty states
   * are allowed so that the state APIs will always be available. Attempts
   * to record to invalid keys will simply be no-ops.
   *
   * @param initialState The JSON object in the "state" property of an addon
   * initialState.
   */
  State(QObject* parent, const QJsonObject& initialState)
      : QObject(parent), m_session(initialState["session"].toObject()) {}
  ~State() = default;

  SessionState* session() { return &m_session; }

 private:
  SessionState m_session;
};

#endif  // STATE_H
