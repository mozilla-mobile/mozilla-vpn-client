/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "tutorialstepbefore.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QMetaMethod>

#include "addons/addontutorial.h"
#include "leakdetector.h"
#include "logger.h"

namespace {
Logger logger("TutorialStepBefore");

class TutorialStepBeforePropertyGet final : public TutorialStepBefore {
 public:
  static TutorialStepBefore* create(AddonTutorial* parent,
                                    const QJsonObject& obj) {
    QString query = obj["query"].toString();
    if (query.isEmpty()) {
      logger.warning() << "Empty 'query' for 'before' step property_get";
      return nullptr;
    }

    QString property = obj["property"].toString();
    if (property.isEmpty()) {
      logger.warning() << "Empty 'property' for 'before' step property_get";
      return nullptr;
    }

    QJsonValue value = obj["value"];
    if (value.type() != QJsonValue::Bool &&
        value.type() != QJsonValue::String &&
        value.type() != QJsonValue::Double) {
      logger.warning() << "Only bool, string and numbers are supported for the "
                          "'before' step property_get";
      return nullptr;
    }

    QVariant valueVariant = value.toVariant();
    if (!valueVariant.isValid()) {
      logger.warning() << "Invalid value for 'before' step property_get";
      return nullptr;
    }

    return new TutorialStepBeforePropertyGet(parent, query, property,
                                             valueVariant);
  };

  TutorialStepBeforePropertyGet(AddonTutorial* parent, const QString& query,
                                const QString& property, const QVariant& value)
      : TutorialStepBefore(parent),
        m_query(query),
        m_property(property),
        m_value(value) {
    MZ_COUNT_CTOR(TutorialStepBeforePropertyGet);
  }

  ~TutorialStepBeforePropertyGet() {
    MZ_COUNT_DTOR(TutorialStepBeforePropertyGet);
  }

  bool run() override {
    QObject* element = nullptr;
    if (!element) {
      return false;
    }

    QVariant property = element->property(qPrintable(m_property));
    if (!property.isValid()) {
      logger.warning() << "Invalid property" << m_property << " for query"
                       << m_query;
      return false;
    }

    return property == m_value;
  }

 private:
  const QString m_query;
  const QString m_property;
  const QVariant m_value;
};

class TutorialStepBeforePropertySet final : public TutorialStepBefore {
 public:
  static TutorialStepBefore* create(AddonTutorial* parent,
                                    const QJsonObject& obj) {
    QString query = obj["query"].toString();
    if (query.isEmpty()) {
      logger.warning() << "Empty 'query' for 'before' step property_set";
      return nullptr;
    }

    QString property = obj["property"].toString();
    if (property.isEmpty()) {
      logger.warning() << "Empty 'property' for 'before' step property_set";
      return nullptr;
    }

    QJsonValue value = obj["value"];
    if (value.type() != QJsonValue::Bool &&
        value.type() != QJsonValue::String &&
        value.type() != QJsonValue::Double) {
      logger.warning() << "Only bool, string and numbers are supported for the "
                          "'before' step property_set";
      return nullptr;
    }

    QVariant valueVariant = value.toVariant();
    if (!valueVariant.isValid()) {
      logger.warning() << "Invalid value for 'before' step property_set";
      return nullptr;
    }

    return new TutorialStepBeforePropertySet(parent, query, property,
                                             valueVariant);
  };

  TutorialStepBeforePropertySet(AddonTutorial* parent, const QString& query,
                                const QString& property, const QVariant& value)
      : TutorialStepBefore(parent),
        m_query(query),
        m_property(property),
        m_value(value) {
    MZ_COUNT_CTOR(TutorialStepBeforePropertySet);
  }

  ~TutorialStepBeforePropertySet() {
    MZ_COUNT_DTOR(TutorialStepBeforePropertySet);
  }

  bool run() override {
    QObject* element = nullptr;
    if (!element) {
      return false;
    }

    element->setProperty(qPrintable(m_property), m_value);
    return true;
  }

 private:
  const QString m_query;
  const QString m_property;
  const QVariant m_value;
};

struct TutorialStepBeforeData {
  QString m_name;
  TutorialStepBefore* (*m_create)(AddonTutorial*, const QJsonObject&);
};

QList<TutorialStepBeforeData> s_stepsBeforeList{
    TutorialStepBeforeData{"property_set",
                           TutorialStepBeforePropertySet::create},
    TutorialStepBeforeData{"property_get",
                           TutorialStepBeforePropertyGet::create},
};
};  // namespace

// static
bool TutorialStepBefore::create(AddonTutorial* parent, const QJsonValue& json,
                                QList<TutorialStepBefore*>& list) {
  QJsonArray array = json.toArray();
  for (const QJsonValue& value : array) {
    QJsonObject obj = value.toObject();

    TutorialStepBefore* tsb = nullptr;
    QString opValue = obj["op"].toString();

    for (const TutorialStepBeforeData& data : s_stepsBeforeList) {
      if (data.m_name == opValue) {
        tsb = data.m_create(parent, obj);
        if (!tsb) {
          logger.warning() << "Unable to create a step 'before' object";
          return false;
        }
        break;
      }
    }

    if (!tsb) {
      logger.warning() << "Invalid 'before' operation:" << opValue;
      return false;
    }

    list.append(tsb);
  }

  return true;
}

// statid
void TutorialStepBefore::registerTutorialStepBefore(
    const QString& name,
    TutorialStepBefore* (*create)(AddonTutorial*, const QJsonObject&)) {
  s_stepsBeforeList.append(TutorialStepBeforeData{name, create});
}

TutorialStepBefore::TutorialStepBefore(AddonTutorial* parent)
    : QObject(parent), m_parent(parent) {
  MZ_COUNT_CTOR(TutorialStepBefore);
}

TutorialStepBefore::~TutorialStepBefore() { MZ_COUNT_DTOR(TutorialStepBefore); }
