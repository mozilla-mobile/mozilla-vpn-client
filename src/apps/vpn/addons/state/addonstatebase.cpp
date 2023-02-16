/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "addonstatebase.h"

#include <QHash>
#include <QJsonObject>

#include "logger.h"

namespace {
Logger logger("AddonStateBase");
}

// static
QJsonValue::Type AddonStateBase::typeToQJsonValueType(QString type) {
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
QHash<QString, QJsonValue> AddonStateBase::parseManifest(
    const QJsonObject& manifest) {
  QHash<QString, QJsonValue> spec;

  foreach (const QString& key, manifest.keys()) {
    QJsonValue value = manifest.value(key);

    if (!value.isObject()) {
      logger.error() << "Addon state value for key" << key
                     << "is incorrectly formatted. Ignoring.";
      continue;
    }

    QJsonValue type = value["type"];
    if (!type.isString()) {
      logger.error() << "Addon state value for key" << key
                     << "has is incorrect 'type' property. Ignoring.";
      continue;
    }

    QJsonValue::Type expectedType = typeToQJsonValueType(type.toString());
    if (expectedType == QJsonValue::Undefined) {
      logger.error() << "Unkown type" << type.toString()
                     << "provided for addon state property.";
      continue;
    }

    QJsonValue def = value["default"];
    if (def.type() != expectedType) {
      logger.error() << "Addon state default value for key" << key
                     << "has incorrectly typed 'default' property. Ignoring.";
      continue;
    }

    spec.insert(key, def);
  }

  return spec;
}

AddonStateBase::AddonStateBase(QHash<QString, QJsonValue> spec)
    : m_defaults(spec) {}

QJsonValue AddonStateBase::get(const QString& key) const {
  if (!m_defaults.contains(key)) {
    logger.error()
        << "Attempted to get key" << key
        << "from addon state, but that key is invalid for this state. ";
    return QJsonValue();
  }

  QJsonValue value = getInternal(key);
  if (value.type() != QJsonValue::Null) return value;

  return m_defaults[key];
}

void AddonStateBase::set(const QString& key, QJsonValue value) {
  if (!m_defaults.contains(key)) {
    logger.error() << "Attempted to set key" << key
                   << "to addon state, but that key is invalid for this state. "
                      "Ignoring.";
    return;
  }

  if (value.type() != m_defaults[key].type()) {
    logger.error() << "Attempted to set state" << key
                   << "to value of incorrect type. Ignoring.";
    return;
  }

  setInternal(key, value);
}

void AddonStateBase::clear(const QString& key) {
  if (key.isEmpty()) {
    clearInternal();
  }

  if (!m_defaults.contains(key)) {
    logger.error()
        << "Attempted to clear key" << key
        << "from addon state, but that key is invalid for this state. "
           "Ignoring.";
    return;
  }

  clearInternal(key);
}
