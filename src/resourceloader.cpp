/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "resourceloader.h"
#include "constants.h"
#include "features/featureremoteresources.h"
#include "leakdetector.h"
#include "logger.h"
#include "networkrequest.h"
#include "resourceloaderutils.h"
#include "task.h"
#include "taskscheduler.h"
#include "update/versionapi.h"

#include <QApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QResource>
#include <QScopeGuard>
#include <QStandardPaths>

constexpr int32_t CONTENT_LENGTH_MAX = 1024 * 1024 * 50;  // more than 50 mb?
constexpr const char* QRB_FOLDER = "qrb";
constexpr const char* QRB_TEMP_FOLDER = "qrb_temp";
constexpr const char* QRB_JOURNAL_FOLDER = "qrb_journal";
constexpr const char* QRB_MANIFEST_JSON = "manifest.json";

namespace {
Logger logger(LOG_MAIN, "ResourceLoader");

class ResourceLoaderTask final : public Task {
 public:
  ResourceLoaderTask() : Task("ResourceLoaderTask") {
    MVPN_COUNT_CTOR(ResourceLoaderTask);
  }

  ~ResourceLoaderTask() { MVPN_COUNT_DTOR(ResourceLoaderTask); }

  void run() override {
    NetworkRequest* request = NetworkRequest::createForRCCUpdates(this);

    connect(request, &NetworkRequest::requestFailed,
            [this](QNetworkReply::NetworkError error, const QByteArray&) {
              logger.error() << "Request failed" << error;
              emit completed();
            });

    connect(request, &NetworkRequest::requestCompleted, this,
            [this](const QByteArray& data) {
              logger.debug() << "Request completed";
              ResourceLoader::instance()->checkForUpdates(data);
              emit completed();
            });
  }
};

bool removeFolderIfExists(QDir& parentDir, const QString& folderName) {
  QDir dir(parentDir.filePath(folderName));
  return !dir.exists() || dir.removeRecursively();
}

}  // namespace

// static
ResourceLoader* ResourceLoader::instance() {
  static ResourceLoader* instance = new ResourceLoader(qApp);
  return instance;
}

ResourceLoader::ResourceLoader(QObject* parent) : QObject(parent) {
  MVPN_COUNT_CTOR(ResourceLoader);

  loadResources();

  if (FeatureRemoteResources::instance()->isSupported()) {
    m_timer.start(Constants::resourceLoaderMsec());
    connect(&m_timer, &QTimer::timeout, this,
            [] { TaskScheduler::scheduleTask(new ResourceLoaderTask()); });
  }
}

ResourceLoader::~ResourceLoader() { MVPN_COUNT_DTOR(ResourceLoader); }

// static
void ResourceLoader::loadResources() {
  auto guard = qScopeGuard([&] {
    QDir dir(MVPN_RESOURCE_PATH);
    if (!dir.exists()) {
      logger.error() << MVPN_RESOURCE_PATH
                     << "does not exit. We are unable to recover. Bad things "
                        "are going to happen.";
      return;
    }

    loadQRB(dir);
  });

  if (!FeatureRemoteResources::instance()->isSupported()) {
    return;
  }

  QDir cacheDir(
      QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
  if (!cacheDir.exists()) {
    logger.info() << "No cache folder found.";
    return;
  }

  if (!cacheDir.cd(QRB_FOLDER) && !recoverMigration(cacheDir)) {
    logger.info() << "Failed to enter in the cache/qrb folder.";
    return;
  }

  if (!validateManifestAndFiles(cacheDir)) {
    return;
  }

  if (!loadQRB(cacheDir)) {
    return;
  }

  guard.dismiss();
}

// static
bool ResourceLoader::recoverMigration(QDir& cacheDir) {
  if (!cacheDir.exists(QRB_JOURNAL_FOLDER)) {
    logger.info() << "No journal folder.";
    return false;
  }

  if (!cacheDir.rename(QRB_JOURNAL_FOLDER, QRB_FOLDER)) {
    logger.info() << "Failed to rename qrb_journal to qrb - recovering";
    return false;
  }

  if (!cacheDir.cd(QRB_FOLDER)) {
    logger.info() << "Failed to enter in the cache/qrb folder after recovering";
    return false;
  }

  logger.info() << "Recovering completed";
  return true;
}

// static
bool ResourceLoader::loadQRB(const QDir& dir) {
  QStringList registeredResources;

  QStringList files = dir.entryList();
  for (const QString& file : files) {
    if (file.startsWith(".") || file == QRB_MANIFEST_JSON) continue;

    if (!file.endsWith(".qrb")) {
      logger.error() << "Unexpected file:" << file;
      continue;
    }

    QString filePath = dir.absoluteFilePath(file);

    logger.debug() << "Loading QT RCC resource:" << filePath;
    if (!QResource::registerResource(filePath)) {
      logger.error() << "Failed to load" << filePath;

      for (const QString& resource : registeredResources) {
        QResource::unregisterResource(resource);
      }

      return false;
    }

    registeredResources.append(filePath);
  }

  return true;
}

void ResourceLoader::checkForUpdates(const QByteArray& inputData) {
  logger.debug() << "Checking for updates";

  // This is where we are going to work.
  QDir cacheDir(
      QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
  if (!cacheDir.exists()) {
    logger.info() << "No cache folder found.";
    return;
  }

  if (!cleanupCacheFolder(cacheDir)) {
    return;
  }

  if (!deserializeData(inputData, cacheDir)) {
    return;
  }

  {
    QDir tmpCacheDir(cacheDir);
    if (!tmpCacheDir.cd(QRB_TEMP_FOLDER)) {
      logger.info() << "Unable to enter into the tmp cache folder.";
      return;
    }

    if (!validateManifestAndFiles(tmpCacheDir)) {
      return;
    }
  }

  bringToProd(cacheDir);
}

bool ResourceLoader::cleanupCacheFolder(QDir& cacheDir) {
  // Let's remove previous operation folders (journal and temp).
  if (!removeFolderIfExists(cacheDir, QRB_JOURNAL_FOLDER)) {
    logger.info() << "Failed to remove a journal folder";
    return false;
  }

  if (!removeFolderIfExists(cacheDir, QRB_TEMP_FOLDER)) {
    logger.info() << "Failed to remove a previous attend to cache data";
    return false;
  }

  // We work on a temporary folder.
  if (!cacheDir.mkdir(QRB_TEMP_FOLDER)) {
    logger.info() << "Failed to create the cache folder";
    return false;
  }

  return true;
}

bool ResourceLoader::deserializeData(const QByteArray& inputData,
                                     const QDir& cacheDir) {
  QDir tmpCacheDir(cacheDir);

  if (!tmpCacheDir.cd(QRB_TEMP_FOLDER)) {
    logger.info() << "Failed to enter into the cache folder";
    return false;
  }

  QByteArray data(inputData);
  if (data.length() < 4) {
    logger.info() << "Invalid format - missing header";
    return false;
  }

  QByteArray header = data.left(4);
  if (header != "MVPN") {
    logger.info() << "Invalid header";
    return false;
  }
  data.remove(0, 4);

  if (data.length() < 1) {
    logger.info() << "Invalid format - missing version";
    return false;
  }

  QByteArray version = data.left(1);
  if (version != QByteArray(1, 0x01)) {
    logger.info() << "Unsupported version";
    return false;
  }
  data.remove(0, 1);

  while (!data.isEmpty()) {
    if (data.length() < 4) {
      logger.info() << "Invalid fileName size.";
      return false;
    }

    QByteArray fileNameLengthBuffer = data.left(4);
    data.remove(0, 4);

    int32_t fileNameLength =
        *reinterpret_cast<const uint32_t*>(fileNameLengthBuffer.constData());
    if (!ResourceLoaderUtils::isFileNameLengthValid(fileNameLength)) {
      logger.info() << "FileNameLength invalid" << fileNameLength;
      return false;
    }

    if (data.length() < fileNameLength) {
      logger.info() << "Failed to read the filename. Not enough data.";
      return false;
    }

    QByteArray fileName = data.left(fileNameLength);
    data.remove(0, fileNameLength);

    if (!ResourceLoaderUtils::isFileNameValid(fileName)) {
      logger.info() << "FileName invalid" << fileName;
      return false;
    }

    QByteArray contentLengthBuffer = data.left(4);
    data.remove(0, 4);

    int32_t contentLength =
        *reinterpret_cast<const uint32_t*>(contentLengthBuffer.constData());
    if (!contentLength || contentLength > CONTENT_LENGTH_MAX) {
      logger.info() << "ContentLength invalid" << contentLength;
      return false;
    }

    if (data.length() < contentLength) {
      logger.info() << "Failed to read the content. Not enough data.";
      return false;
    }

    QFile file(tmpCacheDir.filePath(fileName));
    if (!file.open(QFile::WriteOnly)) {
      logger.info() << "Failed to open" << file.fileName();
      return false;
    }

    if (file.write(data.left(contentLength)) != contentLength) {
      logger.info() << "Failed to write into" << file.fileName();
      return false;
    }

    data.remove(0, contentLength);
  }

  return true;
}

bool ResourceLoader::bringToProd(QDir& cacheDir) {
  // Removing files/directories is a critical, slow, operation. Renaming them
  // is an "atomic" operation on most of the OSs.

  if (cacheDir.exists(QRB_FOLDER) &&
      !cacheDir.rename(QRB_FOLDER, QRB_JOURNAL_FOLDER)) {
    logger.info() << "Failed to rename qrb to qrb_journal";
    return false;
  }

  if (!cacheDir.rename(QRB_TEMP_FOLDER, QRB_FOLDER)) {
    logger.info() << "Failed to rename qrb_temp to qrb";
    return false;
  }

  if (!removeFolderIfExists(cacheDir, QRB_JOURNAL_FOLDER)) {
    logger.info() << "Failed to remove a journal folder after the operation";
    return false;
  }

  return true;
}

// static
bool ResourceLoader::validateManifestAndFiles(const QDir& dir) {
  if (!dir.exists(QRB_MANIFEST_JSON)) {
    logger.error() << "No manifest found!";
    return false;
  }

  // We will use this list to compare the list of resources from the manifest
  // with the content of the directory.
  QStringList files = dir.entryList();
  files.removeOne(".");
  files.removeOne("..");
  files.removeOne(QRB_MANIFEST_JSON);

  QFile manifestFile(dir.absoluteFilePath(QRB_MANIFEST_JSON));
  if (!manifestFile.open(QFile::ReadOnly)) {
    logger.error() << "Failed to open" << QRB_MANIFEST_JSON;
    return false;
  }

  QJsonDocument manifestJson = QJsonDocument::fromJson(manifestFile.readAll());
  if (!manifestJson.isObject()) {
    logger.error() << "A JSON object is expected for the manifest";
    return false;
  }

  QJsonObject manifestObj = manifestJson.object();
  if (manifestObj["version"].toInt() != 1) {
    logger.error() << "Unsupported manifest version";
    return false;
  }

  QString minVersion = manifestObj["minVersion"].toString();
  if (VersionApi::compareVersions(minVersion, APP_VERSION) >= 1) {
    logger.info() << "This minVersion is too high.";
    return false;
  }

  QString maxVersion = manifestObj["maxVersion"].toString();
  if (VersionApi::compareVersions(maxVersion, APP_VERSION) <= -1) {
    logger.info() << "This maxVersion is too low.";
    return false;
  }

  QJsonArray resources = manifestObj["resources"].toArray();
  if (resources.isEmpty()) {
    logger.error() << "No resources in the manifest";
    return false;
  }

  for (QJsonValue resourceValue : resources) {
    QJsonObject resourceObj = resourceValue.toObject();

    QString fileName = resourceObj["fileName"].toString();
    if (fileName.isEmpty()) {
      logger.error() << "Invalid resource fileName in manifest";
      return false;
    }

    if (!files.contains(fileName)) {
      logger.error() << "File" << fileName << "not found.";
      return false;
    }

    QString absoluteFilePath(dir.absoluteFilePath(fileName));
    QFileInfo fileInfo(absoluteFilePath);

    qint64 fileSize = resourceObj["size"].toInt();
    if (fileSize != fileInfo.size()) {
      logger.error() << "File size does not match for file" << fileName
                     << "- expected:" << fileSize
                     << "- found:" << fileInfo.size();
      return false;
    }

    QFile file(absoluteFilePath);
    if (!file.open(QFile::ReadOnly)) {
      logger.error() << "Failed to open" << absoluteFilePath;
      return false;
    }

    QString fileSha256Base64(
        QCryptographicHash::hash(file.readAll(), QCryptographicHash::Sha256)
            .toBase64(QByteArray::Base64Encoding));

    QString sha256Base64 = resourceObj["sha256"].toString();
    if (fileSha256Base64 != sha256Base64) {
      logger.error() << "Sha256 does not match for file" << fileName
                     << "- expected:" << sha256Base64
                     << "- found:" << fileSha256Base64;
      return true;
    }

    files.removeOne(fileName);
  }

  if (!files.isEmpty()) {
    logger.error() << "Extra files found:" << files;
    return false;
  }

  return true;
}
