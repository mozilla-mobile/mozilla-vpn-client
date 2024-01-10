/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "addonindex.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>

#include "addondirectory.h"
#include "addonmanager.h"
#include "constants.h"
#include "leakdetector.h"
#include "logging/logger.h"
#include "settingsholder.h"
#include "signature.h"

namespace {
Logger logger("AddonIndex");
}  // namespace

/**
 * @brief Construct a new AddonIndex::AddonIndex object.
 *
 * The AddonIndex manages both the addons index file and it's signature.
 * The index file contains a list of all of the available addons with respective
 * id and sha256.
 *
 * The main function of this manager is to provide an API to update the index
 * files, get the in memory index of addons, and whenever an update is
 * completed successfully to emit a signal with the updated list of addons.
 */
AddonIndex::AddonIndex(AddonDirectory* addonDirectory) {
  MZ_COUNT_CTOR(AddonIndex);

  Q_ASSERT(addonDirectory);
  m_addonDirectory = addonDirectory;
}

AddonIndex::~AddonIndex() { MZ_COUNT_DTOR(AddonIndex); }

/**
 * @brief Gets the on disk addon list from the addons file, if any.
 *
 * @param addonsList Variable that will contain the index data if reading and
 * validation of data succeeds.
 * @return Whether or not the index file could be read / validated.
 */
bool AddonIndex::getOnDiskAddonsList(QList<AddonData>* addonsList) {
  QByteArray index;
  QByteArray indexSignature;
  if (!read(index, indexSignature)) {
    return false;
  }

  QJsonObject indexObj;
  if (!validate(index, indexSignature, &indexObj)) {
    logger.debug() << "Unable to validate the index";
    return false;
  }

  *addonsList = extractAddonsFromIndex(indexObj);
  return true;
}

/**
 * @brief Reads the index files on disk, if any.
 *
 * @param index Variable that will contain the index data if
 * reading succeeds.
 * @param indexSignature Variable that will contain the index
 * signature if reading succeeds.
 * @returns Whether or not reading of both files was successfull.
 */
bool AddonIndex::read(QByteArray& index, QByteArray& indexSignature) {
  if (!m_addonDirectory->readFile(ADDON_INDEX_FILENAME, &index)) {
    return false;
  }

  if (AddonManager::signatureVerificationNeeded() &&
      !m_addonDirectory->readFile(ADDON_INDEX_SIGNATURE_FILENAME,
                                  &indexSignature)) {
    return false;
  }

  return true;
}

/**
 * @brief Writes new content to the on disk index files.
 *
 * Note: If the addons directory does not exist, this is no-op.
 *
 * @param index The contents to write to the index file.
 * @param indexSignature The contents to write to the index signature file.
 */
bool AddonIndex::write(const QByteArray& index,
                       const QByteArray& indexSignature) {
  if (!m_addonDirectory->writeToFile(ADDON_INDEX_FILENAME, index)) {
    return false;
  }

  if (AddonManager::signatureVerificationNeeded() &&
      !m_addonDirectory->writeToFile(ADDON_INDEX_SIGNATURE_FILENAME,
                                     indexSignature)) {
    return false;
  }

  return true;
}

/**
 * @brief Updates the index on file in case provided index and indexSignature
 * are valid and different from the currently stored ones.
 *
 * Note: If update is successful `indexUpdated` signal is emited. If nothing has
 * changed, this signal is not emited.
 *
 * @param index The new index data to update.
 * @param indexSignature The new index signature to update.
 * @param ignoreUnchanged Whether or not to ignore the fact that the index has
 * not changed. Defaults to `false`.
 * @return Whether or not an update signal was emited.
 */
void AddonIndex::update(const QByteArray& index,
                        const QByteArray& indexSignature) {
  QByteArray currentIndex;
  QByteArray currentIndexSignature;
  if (read(currentIndex, currentIndexSignature) && currentIndex == index &&
      currentIndexSignature == indexSignature) {
    logger.debug() << "The index has not changed";
    emit indexUpdated(false, QList<AddonData>());
    return;
  }

  QJsonObject indexObj;
  if (!validate(index, indexSignature, &indexObj)) {
    logger.debug() << "Unable to validate the index";
    emit indexUpdated(false, QList<AddonData>());
    return;
  }

  if (!write(index, indexSignature)) {
    logger.debug() << "Unable to write to index file";
    emit indexUpdated(false, QList<AddonData>());
    return;
  }

  QList<AddonData> addons = extractAddonsFromIndex(indexObj);
  emit indexUpdated(true, addons);
}

/**
 * @brief Validates the addon files. That includes validation of the signature
 * as well as of the format of the JSON in the index file.
 *
 * Note: If the addons directory does not exist, this is no-op.
 *
 * @param index The contents of the index file.
 * @param indexSignature The contents of the index signature file.
 * @returns Whether or not validation was succesfull.
 */
// static
bool AddonIndex::validate(const QByteArray& index,
                          const QByteArray& indexSignature,
                          QJsonObject* indexObj) {
  if (!validateIndex(index, indexObj)) {
    return false;
  }

  if (AddonManager::signatureVerificationNeeded() &&
      !validateIndexSignature(index, indexSignature)) {
    return false;
  }

  return true;
}

bool AddonIndex::validateIndex(const QByteArray& index, QJsonObject* indexObj) {
  QJsonDocument doc = QJsonDocument::fromJson(index);
  if (!doc.isObject()) {
    logger.debug() << "The index must be an object";
    return false;
  }

  QJsonObject obj = doc.object();
  if (obj["api_version"].toString() != ADDONS_API_VERSION) {
    logger.debug() << "Invalid index file - api_version does not match";
    return false;
  }

  const QJsonArray addons = obj["addons"].toArray();
  for (const QJsonValue& item : addons) {
    if (!validateSingleAddonIndex(item)) {
      return false;
    }
  }

  *indexObj = obj;
  return true;
}

// static
bool AddonIndex::validateSingleAddonIndex(const QJsonValue& addonValue) {
  if (!addonValue.isObject()) {
    logger.warning() << "Addon item is not a JSON object";
    return false;
  }

  QJsonObject addonObj = addonValue.toObject();

  QString sha256hex = addonObj["sha256"].toString();
  if (sha256hex.isEmpty()) {
    logger.warning() << "Incomplete index - sha256";
    return false;
  }

  if (sha256hex.length() != 64) {
    logger.warning() << "Invalid sha256 hash";
    return false;
  }

  if (addonObj["id"].toString().isEmpty()) {
    logger.warning() << "Incomplete index - addonId";
    return false;
  }

  return true;
}

// static
bool AddonIndex::validateIndexSignature(const QByteArray& index,
                                        const QByteArray& indexSignature) {
  QString publicKeyUrl(Constants::ADDON_PRODUCTION_KEY);
  if (!Constants::inProduction() &&
      !SettingsHolder::instance()->addonProdKeyInStaging()) {
    publicKeyUrl = Constants::ADDON_STAGING_KEY;
  }

  QFile publicKeyFile(publicKeyUrl);
  if (!publicKeyFile.open(QIODevice::ReadOnly)) {
    logger.warning() << "Unable to open the addon public key file";
    return false;
  }

  QByteArray publicKey = publicKeyFile.readAll();
  if (!Signature::verify(publicKey, index, indexSignature)) {
    logger.warning() << "Unable to verify the signature of the addon index";
    return false;
  }

  return true;
}

// static
QList<AddonData> AddonIndex::extractAddonsFromIndex(
    const QJsonObject& indexObj) {
  QList<AddonData> addons;
  const QJsonArray addonArray = indexObj["addons"].toArray();
  for (const QJsonValue& item : addonArray) {
    QJsonObject addon = item.toObject();
    addons.append(
        {QByteArray::fromHex(addon["sha256"].toString().toLocal8Bit()),
         addon["id"].toString(), nullptr});
  }

  return addons;
}
