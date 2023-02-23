/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SESSIONSTATE_H
#define SESSIONSTATE_H

#include "statebase.h"

/**
 * @brief Session State implementation,
 * in which state does not persist throughout restarts.
 */
class SessionState final : public StateBase {
 public:
  ~SessionState();

  /**
   * @brief Construct an SessionState object from the JSON object under the
   * `state` key of an Addon's manifest.
   *
   * The manifest should be in the following format:
   *
   * ```
   * {
   *    [key: string]: {
   *      "type": "boolean" | "string" | "number",
   *      "default": boolean | string | number
   *    }
   * }
   * ```
   *
   * Invalid properties on the manifest will be ignored. In the worst case
   * if the manifest is completely wrong an empty state will be generated.
   *
   * @param manifest The addon's manifest.
   * @return State the generated state.
   */
  SessionState(const QJsonObject& manifest);

 private:
  QJsonValue getInternal(const QString& key) const override;
  void setInternal(const QString& key, const QJsonValue& value) override;
  void clearInternal(const QString& key = "") override;
  StateHash m_state;
};

#endif  // SESSIONSTATE_H
