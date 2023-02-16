/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADDONSTATEBASE_H
#define ADDONSTATEBASE_H

#include <QHash>
#include <QJsonValue>
#include <QObject>
#include <QString>

typedef QHash<QString, QJsonValue> StateHash;

/**
 * @brief Abstract base class describing the state of an addon.
 *
 * The state of an addon is a collection of properties that can change how the
 * addon is presented or how it behaves.
 *
 * A given addon's state is defined in it's manifest under the `state` key. In
 * practice, it is a dictionary of key/value pairs. The keys are strings and the
 * values may be either boolean, numbers or strings. The keys are not dinamic.
 * The values can be changed, but the kays are limited to whatever is defined in
 * the addon manifest. The type of the value cannot be changed.
 */
class AddonStateBase : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AddonStateBase)

 public:
  ~AddonStateBase() = default;

  /**
   * @brief Get the value for a given key in the state.
   *
   * If key was not defined in the manifest,
   * QJsonValue::Undefined will be returned.
   *
   * If the key has never been written to, the default value will be
   * returned.
   *
   * @return QJsonValue The stored value for the given key. A nullptr is
   * returned if the key is invalid i.e. not provided on the manifest on
   * init.
   */
  Q_INVOKABLE QJsonValue get(const QString& key) const;

  /**
   * @brief Sets the value for a given key in the addon state.
   *
   * If key was not defined in the manifest, operation will be ignored.
   *
   * `value` must be of the type defined in the manifest,
   * otherwise it will be ignored.
   *
   * @param key The key to set.
   * @param value The value to set.
   */
  Q_INVOKABLE void set(const QString& key, QJsonValue value);

  /**
   * @brief Clears stored values in the state.
   *
   * If key was not defined in the manifest, operation will be ignored.
   *
   * If the key has never been written to, this is a no-op.
   *
   * @param key The key to clear.
   */
  Q_INVOKABLE void clear(const QString& key = "");

 protected:
  AddonStateBase(StateHash spec);
  StateHash m_defaults;

  static StateHash parseManifest(const QJsonObject& manifest);

  // Methods to override.
  virtual QJsonValue getInternal(const QString& key) const = 0;
  virtual void setInternal(const QString& key, const QJsonValue& value) = 0;
  virtual void clearInternal(const QString& key = "") = 0;

 private:
  static QJsonValue::Type typeToQJsonValueType(QString type);
};

#endif  // ADDONSTATEBASE_H
