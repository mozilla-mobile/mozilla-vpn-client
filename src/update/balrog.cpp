/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "balrog.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QProcess>
#include <QScopeGuard>
#include <QSslCertificate>
#include <QSslKey>

#include "constants.h"
#include "errorhandler.h"
#include "glean/generated/metrics.h"
#include "leakdetector.h"
#include "logger.h"
#include "networkrequest.h"

// Implemented in rust. See the `signature` folder.
// TODO: We should really generate this with cbindgen.
extern "C" {
bool verify_content_signature(const char* x5u_ptr, size_t x5u_length,
                              const char* msg_ptr, size_t msg_length,
                              const char* signature, const char* rootHash,
                              const char* certSubject,
                              void (*logfn)(const char*));
}

#if defined(MZ_WINDOWS)
constexpr const char* BALROG_WINDOWS_UA = "WINNT_x86_64";
#elif defined(MZ_MACOS)
constexpr const char* BALROG_MACOS_UA = "Darwin_x86";
#else
#  error Platform not supported yet
#endif

constexpr const char* BALROG_CERT_SUBJECT_CN =
    "aus.content-signature.mozilla.org";

namespace {
Logger logger("Balrog");

static void balrogLogger(const char* msg) {
  logger.debug() << msg;
}

}  // namespace

Balrog::Balrog(QObject* parent, bool downloadAndInstall,
               ErrorHandler::ErrorPropagationPolicy errorPropagationPolicy)
    : Updater(parent),
      m_downloadAndInstall(downloadAndInstall),
      m_errorPropagationPolicy(errorPropagationPolicy) {
  MZ_COUNT_CTOR(Balrog);
  logger.debug() << "Balrog created";
}

Balrog::~Balrog() {
  MZ_COUNT_DTOR(Balrog);
  logger.debug() << "Balrog released";
}

// static
QString Balrog::userAgent() {
#if defined(MZ_WINDOWS)
  return BALROG_WINDOWS_UA;
#elif defined(MZ_MACOS)
  return BALROG_MACOS_UA;
#else
#  error Unsupported platform
#endif
}

void Balrog::start(Task* task) {
  if (m_downloadAndInstall) {
    mozilla::glean::sample::update_step.record(
        mozilla::glean::sample::UpdateStepExtra{
            ._state = QVariant::fromValue(UpdateProcessStarted).toString()});
  }

  QString url =
      QString(Constants::balrogUrl()).arg(appVersion()).arg(userAgent());
  logger.debug() << "URL:" << url;

  NetworkRequest* request = new NetworkRequest(task, 200);
  request->get(url);

  connect(request, &NetworkRequest::requestFailed, this,
          [this](QNetworkReply::NetworkError error, const QByteArray&) {
            logger.error() << "Request failed" << error;
            deleteLater();
          });

  connect(request, &NetworkRequest::requestCompleted, this,
          [this, task, request](const QByteArray& data) {
            logger.debug() << "Request completed";

            if (!fetchSignature(task, request, data)) {
              logger.warning() << "Ignore failure.";
              deleteLater();
            }
          });
}

bool Balrog::fetchSignature(Task* task, NetworkRequest* initialRequest,
                            const QByteArray& updateData) {
  Q_ASSERT(initialRequest);

  QByteArray header = initialRequest->rawHeader("Content-Signature");
  if (header.isEmpty()) {
    logger.error() << "Content-Signature missing";
    return false;
  }

  QByteArray x5u;
  QByteArray signatureBlob;

  for (const QByteArray& item : header.split(';')) {
    QByteArray entry = item.trimmed();
    int pos = entry.indexOf('=');
    if (pos == -1) {
      logger.error() << "Invalid entry:" << item;
      return false;
    }

    QByteArray key = entry.left(pos);
    if (key == "x5u") {
      x5u = entry.remove(0, pos + 1);
    } else {
      signatureBlob = entry.remove(0, pos + 1);
    }
  }

  if (x5u.isEmpty() || signatureBlob.isEmpty()) {
    logger.error() << "No signatureBlob or x5u found";
    return false;
  }

  logger.debug() << "Fetching x5u URL:" << x5u;

  NetworkRequest* x5uRequest = new NetworkRequest(task, 200);
  x5uRequest->get(QString(x5u));

  connect(x5uRequest, &NetworkRequest::requestFailed, this,
          [this](QNetworkReply::NetworkError error, const QByteArray&) {
            logger.warning() << "Request failed" << error;
            deleteLater();
          });

  connect(x5uRequest, &NetworkRequest::requestCompleted, this,
          [this, task, signatureBlob, updateData](const QByteArray& x5uData) {
            logger.debug() << "Request completed";
            if (!checkSignature(task, x5uData, updateData, signatureBlob)) {
              deleteLater();
            }
          });

  return true;
}

bool Balrog::checkSignature(Task* task, const QByteArray& x5uData,
                            const QByteArray& updateData,
                            const QByteArray& signatureBlob) {
  logger.debug() << "Checking the signature";

  if (!validateSignature(x5uData, updateData, signatureBlob)) {
    logger.error() << "Invalid signature";
    return false;
  }

  logger.debug() << "Fetch resource";
  if (!processData(task, updateData)) {
    logger.error() << "Fetch has failed";
    return false;
  }

  return true;
}

bool Balrog::validateSignature(const QByteArray& x5uData,
                               const QByteArray& updateData,
                               const QByteArray& signatureBlob) {
  bool verify = verify_content_signature(
      x5uData.constData(), x5uData.length(), updateData.constData(),
      updateData.length(), signatureBlob.constData(),
      Constants::AUTOGRAPH_ROOT_CERT_FINGERPRINT, BALROG_CERT_SUBJECT_CN,
      balrogLogger);
  if (!verify) {
    logger.error() << "Verification failed";
    return false;
  } else {
    logger.info() << "Verification succeeded";
    return true;
  }
}

bool Balrog::processData(Task* task, const QByteArray& data) {
  QJsonDocument json = QJsonDocument::fromJson(data);
  if (!json.isObject()) {
    logger.error() << "A valid JSON object expected";
    return false;
  }

  QJsonObject obj = json.object();

  // This is not a download operation. Let's emit signals if needed.
  if (!m_downloadAndInstall) {
    bool required = obj.value("required").toBool();
    if (required) {
      // Even if we are geoip-restricted, we want to force the update when
      // required.
      emit updateRequired();
      deleteLater();
      return true;
    }

    // Let's see if we can fetch the content. If we are unable to fetch the
    // content, we don't push for a recommended update.
    QString url = obj.value("url").toString();
    if (url.isEmpty()) {
      logger.error() << "Invalid URL in the JSON document";
      return false;
    }

    NetworkRequest* request = new NetworkRequest(task);
    request->get(url);

    connect(request, &NetworkRequest::requestFailed, this,
            [this](QNetworkReply::NetworkError error, const QByteArray&) {
              logger.error() << "Request failed" << error;
              deleteLater();
            });

    connect(request, &NetworkRequest::requestHeaderReceived, this,
            [this](NetworkRequest* request) {
              Q_ASSERT(request);
              logger.debug() << "Request header received";

              // We want to proceed only if the status code is 200. The request
              // will be aborted, but the signal emitted.
              if (request->statusCode() == 200) {
                emit updateRecommended();
              }

              logger.warning()
                  << "Abort request for status code" << request->statusCode();
              request->abort();
            });

    connect(request, &NetworkRequest::requestCompleted, this,
            [this](const QByteArray&) {
              logger.debug() << "Request completed";
              deleteLater();
            });

    return true;
  }

  QString url = obj.value("url").toString();
  if (url.isEmpty()) {
    logger.error() << "Invalid URL in the JSON document";
    return false;
  }

  QString hashFunction = obj.value("hashFunction").toString();
  if (hashFunction.isEmpty()) {
    logger.error() << "No hashFunction item";
    return false;
  }

  QString hashValue = obj.value("hashValue").toString();
  if (hashValue.isEmpty()) {
    logger.error() << "No hashValue item";
    return false;
  }

  NetworkRequest* request = new NetworkRequest(task);
  request->get(url);

  // No timeout for this request.
  request->disableTimeout();

  connect(
      request, &NetworkRequest::requestFailed, this,
      [this, request](QNetworkReply::NetworkError error, const QByteArray&) {
        logger.error() << "Request failed" << error;
        propagateError(request, error);
        deleteLater();
      });

  connect(request, &NetworkRequest::requestCompleted, this,
          [this, hashValue, hashFunction, url](const QByteArray& data) {
            logger.debug() << "Request completed";

            if (!computeHash(url, data, hashValue, hashFunction)) {
              logger.error() << "Ignore failure.";
              deleteLater();
            }
          });

  return true;
}

bool Balrog::computeHash(const QString& url, const QByteArray& data,
                         const QString& hashValue,
                         const QString& hashFunction) {
  logger.debug() << "Compute the hash";

  if (hashFunction != "sha512") {
    logger.error() << "Invalid hash function";
    return false;
  }

  QCryptographicHash hash(QCryptographicHash::Sha512);
  hash.addData(data);
  QByteArray hashHex = hash.result().toHex();

  if (hashHex != hashValue) {
    logger.error() << "Hash doesn't match";
    return false;
  }

  mozilla::glean::sample::update_step.record(
      mozilla::glean::sample::UpdateStepExtra{
          ._state = QVariant::fromValue(BalrogValidationCompleted).toString()});

  return saveFileAndInstall(url, data);
}

bool Balrog::saveFileAndInstall(const QString& url, const QByteArray& data) {
  logger.debug() << "Save the file and install it";

  int pos = url.lastIndexOf("/");
  if (pos == -1) {
    logger.error() << "The URL seems to be without /.";
    return false;
  }

  QString fileName = url.right(url.length() - pos - 1);
  logger.debug() << "Filename:" << fileName;

  if (!m_tmpDir.isValid()) {
    logger.error() << "Cannot create a temporary directory"
                   << m_tmpDir.errorString();
    return false;
  }

  QDir dir(m_tmpDir.path());
  QString tmpFile = dir.filePath(fileName);

  QFile file(tmpFile);
  if (!file.open(QIODevice::ReadWrite)) {
    logger.error() << "Unable to create a file in the temporary folder";
    return false;
  }

  qint64 written = file.write(data);
  if (written != data.length()) {
    logger.error() << "Unable to write the whole configuration file";
    return false;
  }

  file.close();

  mozilla::glean::sample::update_step.record(
      mozilla::glean::sample::UpdateStepExtra{
          ._state = QVariant::fromValue(BalrogFileSaved).toString()});

  return install(tmpFile);
}

bool Balrog::install(const QString& filePath) {
  logger.debug() << "Install the package:" << filePath;

  QString logFile = m_tmpDir.filePath("msiexec.log");

#if defined(MZ_WINDOWS)
  QStringList arguments;
  arguments << "/qb!-"
            << "REBOOT=ReallySuppress"
            << "/i" << QDir::toNativeSeparators(filePath) << "/lv!"
            << QDir::toNativeSeparators(logFile);

  QProcess* process = new QProcess(this);
  process->setProgram("msiexec.exe");
  process->setArguments(arguments);

  connect(process, &QProcess::readyReadStandardError, [process]() {
    logger.info() << "[msiexec - stderr]" << Qt::endl
                  << qUtf8Printable(process->readAllStandardError())
                  << Qt::endl;
  });

  connect(process, &QProcess::readyReadStandardOutput, [process]() {
    logger.info() << "[msiexec - stdout]" << Qt::endl
                  << qUtf8Printable(process->readAllStandardOutput())
                  << Qt::endl;
  });

  connect(process, &QProcess::errorOccurred, this,
          [this](QProcess::ProcessError error) {
            logger.error() << "Installer execution failed:" << error;
            deleteLater();
          });

  if (!process->startDetached()) {
    logger.error() << "Unable to run the installer";
    deleteLater();
  }
#endif

#if defined(MZ_MACOS)
  QStringList arguments;
  arguments << filePath;

  QProcess* process = new QProcess(this);
  process->start("open", arguments);
  connect(
      process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
      this, [this, process](int exitCode, QProcess::ExitStatus) {
        logger.info() << "Async installation activated - exitCode:" << exitCode;

        logger.info() << "Stdout:" << Qt::endl
                      << qUtf8Printable(process->readAllStandardOutput())
                      << Qt::endl;
        logger.info() << "Stderr:" << Qt::endl
                      << qUtf8Printable(process->readAllStandardError())
                      << Qt::endl;

        if (exitCode != 0) {
          deleteLater();
          return;
        }

        // We leak the object because the installer will restart the
        // app and we need to keep the temporary folder alive during the
        // whole process.
        exit(0);
      });
#endif

  mozilla::glean::sample::update_step.record(
      mozilla::glean::sample::UpdateStepExtra{
          ._state =
              QVariant::fromValue(InstallationProcessExecuted).toString()});

  return true;
}

void Balrog::propagateError(NetworkRequest* request,
                            QNetworkReply::NetworkError error) {
  Q_ASSERT(request);

  // 451 Unavailable For Legal Reasons
  if (request->statusCode() == 451) {
    logger.debug() << "Geo IP restriction detected";
    REPORTERROR(ErrorHandler::GeoIpRestrictionError, "balrog");
    return;
  }

  REPORTNETWORKERROR(error, m_errorPropagationPolicy, "balrog");
}
