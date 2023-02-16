/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADDONSESSIONSTATE_H
#define ADDONSESSIONSTATE_H

#include <QHash>
#include <QJsonValue>
#include <QObject>
#include <QString>

#include "addonstatebase.h"

/**
 * @brief Addon State implementation,
 * in which state does not persist throughout restarts.
 */
class AddonSessionState final : public AddonStateBase {
 public:
  ~AddonSessionState();
  AddonSessionState(StateHash spec);

  /**
   * @brief Construct an AddonSessionState object from the JSON object under the
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
   * Invalid properties on the manifest will be ignored. In the worst case if
   * the manifest is completely wrong an empty state will be generated.
   *
   * @param manifest The addon's manifest.
   * @return AddonState the generated state.
   */
  static AddonSessionState* fromManifest(const QJsonObject& manifest);

 protected:
  QJsonValue getInternal(const QString& key) const override;
  void setInternal(const QString& key, const QJsonValue& value) override;
  void clearInternal(const QString& key = "") override;

 private:
  StateHash m_state;
};

#endif  // ADDONSESSIONSTATE_H
