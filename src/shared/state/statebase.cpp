/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "statebase.h"

#include <QJsonObject>

#include "logger.h"

namespace {
Logger logger("StateBase");
}

// static
QJsonValue::Type StateBase::typeToQJsonValueType(const QString& type) {
  if (type == "boolean") {
    return QJsonValue::Bool;
  }

  if (type == "number") {
    return QJsonValue::Double;
  }

  if (type == "string") {
    return QJsonValue::String;
  }

  return QJsonValue::Undefined;
}

// static
StateHash StateBase::parseAddonManifest(const QJsonObject& initialState) {
  StateHash spec;

  foreach (const QString& key, initialState.keys()) {
    QJsonValue value = initialState.value(key);

    if (!value.isObject()) {
      logger.error() << "State value for key" << key
                     << "is incorrectly formatted. Ignoring.";
      continue;
    }

    QJsonValue type = value["type"];
    if (!type.isString()) {
      logger.error() << "State value for key" << key
                     << "has is incorrect 'type' property. Ignoring.";
      continue;
    }

    QJsonValue::Type expectedType = typeToQJsonValueType(type.toString());
    if (expectedType == QJsonValue::Undefined) {
      logger.error() << "Unkown type" << type.toString()
                     << "provided for state property.";
      continue;
    }

    QJsonValue def = value["default"];
    if (def.type() != expectedType) {
      logger.error() << "State default value for key" << key
                     << "has incorrectly typed 'default' property. Ignoring.";
      continue;
    }

    spec.insert(key, def);
  }

  return spec;
}

StateBase::StateBase(const QJsonObject& spec)
    : m_defaults(StateBase::parseAddonManifest(spec)) {}

QJsonValue StateBase::get(const QString& key) {
  if (!m_defaults.contains(key)) {
    logger.error() << "Attempted to get key" << key
                   << "from state, but that key is invalid for this property. ";
    return QJsonValue();
  }

  QJsonValue value = getInternal(key);

  if (value.type() == m_defaults[key].type()) {
    return value;
  }

  if (value.type() != QJsonValue::Null) {
    logger.error()
        << "Attempted to get key" << key
        << "from state, but the stored value is of unexpected type. Clearing.";

    clear(key);
  }

  return m_defaults[key];
}

void StateBase::set(const QString& key, QJsonValue value) {
  if (!m_defaults.contains(key)) {
    logger.error() << "Attempted to set key" << key
                   << "to state, but that key is invalid for this property. "
                      "Ignoring.";
    return;
  }

  if (value.type() != m_defaults[key].type()) {
    logger.error() << "Attempted to set property" << key
                   << "to value of incorrect type. Ignoring.";
    return;
  }

  setInternal(key, value);
}

void StateBase::clear(const QString& key) {
  if (key.isEmpty()) {
    clearInternal();
    return;
  }

  if (!m_defaults.contains(key)) {
    logger.error() << "Attempted to clear key" << key
                   << "from state, but that key is invalid for this state. "
                      "Ignoring.";
    return;
  }

  clearInternal(key);
}
