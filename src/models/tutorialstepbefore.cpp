/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "tutorialstepbefore.h"
#include "inspector/inspectorutils.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "settingsholder.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QMetaMethod>

namespace {
Logger logger(LOG_MAIN, "TutorialStepBefore");
}

// static
QList<TutorialStepBefore*> TutorialStepBefore::create(
    QObject* parent, const QString& elementForTooltip, const QJsonValue& json) {
  QList<TutorialStepBefore*> list;

  for (QJsonValue value : json.toArray()) {
    QJsonObject obj = value.toObject();

    Op op;
    QString opValue = obj["op"].toString();
    if (opValue == "property_set") {
      op = PropertySet;
    } else if (opValue == "property_get") {
      op = PropertyGet;
    } else {
      logger.warning() << "Invalid 'before' operation:" << opValue;
      return QList<TutorialStepBefore*>();
    }

    QString element = obj["element"].toString();
    if (element.isEmpty()) {
      logger.warning() << "Empty element property for 'before' step";
      return QList<TutorialStepBefore*>();
    }

    QString property = obj["property"].toString();
    if (property.isEmpty()) {
      logger.warning() << "Empty property property for 'before' step";
      return QList<TutorialStepBefore*>();
    }

    QJsonValue propertyValue = obj["value"];
    if (propertyValue.type() != QJsonValue::Bool &&
        propertyValue.type() != QJsonValue::String &&
        propertyValue.type() != QJsonValue::Double) {
      logger.warning() << "Only bool, string and numbers are supported for the "
                          "'before' step value";
      return QList<TutorialStepBefore*>();
    }

    QVariant valueVariant = propertyValue.toVariant();
    if (!valueVariant.isValid()) {
      logger.warning() << "Invalid value for 'before' step";
      return QList<TutorialStepBefore*>();
    }

    list.append(
        new TutorialStepBefore(parent, op, element, property, valueVariant));
  }

  // The tooltip element must be visible.
  list.prepend(new TutorialStepBefore(parent, PropertyGet, elementForTooltip,
                                      "visible", QVariant(true)));
  return list;
}

TutorialStepBefore::TutorialStepBefore(QObject* parent, Op op,
                                       const QString& element,
                                       const QString& property,
                                       const QVariant& value)
    : QObject(parent),
      m_op(op),
      m_element(element),
      m_property(property),
      m_value(value) {
  MVPN_COUNT_CTOR(TutorialStepBefore);
}

TutorialStepBefore::~TutorialStepBefore() {
  MVPN_COUNT_DTOR(TutorialStepBefore);
}

bool TutorialStepBefore::run() {
  switch (m_op) {
    case PropertySet:
      return runPropertySet();

    case PropertyGet:
      return runPropertyGet();

    default:
      Q_UNREACHABLE();
  }
}

bool TutorialStepBefore::runPropertyGet() {
  QObject* element = InspectorUtils::findObject(m_element);
  if (!element) {
    return false;
  }

  QVariant property = element->property(qPrintable(m_property));
  if (!property.isValid()) {
    logger.warning() << "Invalid property" << m_property << " for element"
                     << m_element;
    return false;
  }

  return property == m_value;
}

bool TutorialStepBefore::runPropertySet() {
  QObject* element = InspectorUtils::findObject(m_element);
  if (!element) {
    return false;
  }

  element->setProperty(qPrintable(m_property), m_value);
  return true;
}
