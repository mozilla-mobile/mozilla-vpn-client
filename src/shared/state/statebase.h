/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef STATEBASE_H
#define STATEBASE_H

#include <QHash>
#include <QJsonValue>
#include <QString>

typedef QHash<QString, QJsonValue> StateHash;

/**
 * @brief Abstract base class describing the state of something.
 *
 * Although this class' `get` and `set` APIs return the generic QJsonValue type,
 * the types are validated based on whatever is provided as a default on
 * initialize.
 *
 * 1. When "something" is an addon:
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
class StateBase {
  Q_GADGET

 public:
  virtual ~StateBase() = default;

  /**
   * @brief Get the value for a given key in the state.
   *
   * If key was not defined in the initial state,
   * QJsonValue::Undefined will be returned.
   *
   * If the key has never been written to, the default value will be
   * returned.
   *
   * This is not a const function, because in case a malformatted (i.e. a value
   * this is not of the same type as the default provided) value is found in the
   * storage it will be deleted.
   *
   * @return QJsonValue The stored value for the given key. A nullptr is
   * returned if the key is invalid i.e. not provided on the initial state on
   * init.
   */
  Q_INVOKABLE QJsonValue get(const QString& key);

  /**
   * @brief Sets the value for a given key in the addon state.
   *
   * If key was not defined in the initial state, operation will be ignored.
   *
   * `value` must be of the type defined in the initial state,
   * otherwise it will be ignored.
   *
   * @param key The key to set.
   * @param value The value to set.
   */
  Q_INVOKABLE void set(const QString& key, QJsonValue value);

  /**
   * @brief Clears stored values in the state.
   *
   * If key was not defined in the initial state, operation will be ignored.
   *
   * If the key has never been written to, this is a no-op.
   *
   * @param key The key to clear.
   */
  Q_INVOKABLE void clear(const QString& key = "");

 protected:
  explicit StateBase(const QJsonObject& spec);
  explicit StateBase(const StateHash& spec) : m_defaults(spec) {}
  StateHash m_defaults;

  // Methods to override.
  virtual QJsonValue getInternal(const QString& key) const = 0;
  virtual void setInternal(const QString& key, const QJsonValue& value) = 0;
  virtual void clearInternal(const QString& key = "") = 0;

 private:
  static QJsonValue::Type typeToQJsonValueType(const QString& type);
  static StateHash parseAddonManifest(const QJsonObject& initialState);
};

#endif  // STATEBASE_H
