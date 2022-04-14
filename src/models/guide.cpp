/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "guide.h"
#include "guideblock.h"
#include "l18nstrings.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/feature.h"
#include "mozillavpn.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QScopeGuard>

namespace {
Logger logger(LOG_MAIN, "Guide");

}

Guide::Guide(QObject* parent) : QObject(parent) { MVPN_COUNT_CTOR(Guide); }

Guide::~Guide() { MVPN_COUNT_DTOR(Guide); }

// static
Guide* Guide::create(QObject* parent, const QString& fileName) {
  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    logger.error() << "Unable to read the guide file" << fileName;
    return nullptr;
  }

  QByteArray content = file.readAll();
  QJsonDocument json = QJsonDocument::fromJson(content);
  if (!json.isObject()) {
    logger.error() << "Invalid JSON file" << fileName;
    return nullptr;
  }

  QJsonObject obj = json.object();

  QJsonObject conditions = obj["conditions"].toObject();
  if (!evaluateConditions(conditions)) {
    logger.info() << "Exclude the guide because conditions do not match";
    return nullptr;
  }

  QString guideId = obj["id"].toString();
  if (guideId.isEmpty()) {
    logger.error() << "Empty ID for guide file" << fileName;
    return nullptr;
  }

  L18nStrings* l18nStrings = L18nStrings::instance();
  Q_ASSERT(l18nStrings);

  Guide* guide = new Guide(parent);
  auto guard = qScopeGuard([&] { guide->deleteLater(); });

  guide->m_id = pascalize(QString("guide_%1_title").arg(guideId));
  if (!l18nStrings->contains(guide->m_id)) {
    logger.error() << "No string ID found for the title of guide file"
                   << fileName << "ID:" << guide->m_id;
    return nullptr;
  }

  guide->m_image = obj["image"].toString();
  if (guide->m_image.isEmpty()) {
    logger.error() << "Empty image for guide file" << fileName;
    return nullptr;
  }

  QJsonValue blocksArray = obj["blocks"];
  if (!blocksArray.isArray()) {
    logger.error() << "No blocks for guide file" << fileName;
    return nullptr;
  }

  for (QJsonValue blockValue : blocksArray.toArray()) {
    if (!blockValue.isObject()) {
      logger.error() << "Expected JSON objects as blocks for guide file"
                     << fileName;
      return nullptr;
    }

    QJsonObject blockObj = blockValue.toObject();

    GuideBlock* block = GuideBlock::create(guide, guideId, fileName, blockObj);
    if (!block) {
      return nullptr;
    }

    guide->m_blocks.append(block);
  }

  guard.dismiss();
  return guide;
}

// static
QString Guide::pascalize(const QString& input) {
  QString output;

  for (QString chunk : input.split("_")) {
    if (chunk.isEmpty()) continue;

    chunk = chunk.toLower();
    chunk[0] = chunk[0].toUpper();
    output.append(chunk);
  }

  return output;
}

// static
bool Guide::evaluateConditions(const QJsonObject& conditions) {
  for (QJsonValue enabledFeature : conditions["enabledFeatures"].toArray()) {
    // If the feature doesn't exist, we crash.
    const Feature* feature = Feature::get(enabledFeature.toString());
    Q_ASSERT(feature);

    if (!feature->isSupported()) {
      logger.info() << "Feature not supported";
      return false;
    }
  }

  QStringList platforms;
  for (QJsonValue platform : conditions["platforms"].toArray()) {
    platforms.append(platform.toString());
  }

  if (!platforms.isEmpty() &&
      !platforms.contains(MozillaVPN::instance()->platform())) {
    logger.info() << "Not supported platform";
    return false;
  }

  return true;
}
