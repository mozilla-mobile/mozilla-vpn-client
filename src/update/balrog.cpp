/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "balrog.h"
#include "constants.h"
#include "errorhandler.h"
#include "inspector/inspectorwebsocketconnection.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkrequest.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QProcess>
#include <QScopeGuard>
#include <QSslCertificate>
#include <QSslKey>
#include <QTemporaryDir>

typedef struct {
  const char* p;
  size_t n;
} gostring_t;

typedef void (*logFunc)(int level, const char* msg);

#if defined(MVPN_WINDOWS)
#  include "windows.h"
#  include "platforms/windows/windowscommons.h"

constexpr const char* BALROG_WINDOWS_UA64 = "WINNT_x86_64";
constexpr const char* BALROG_WINDOWS_UA32 = "WINNT_x86_32";

typedef void BalrogSetLogger(logFunc func);
typedef unsigned char BalrogValidateSignature(gostring_t publicKey,
                                              gostring_t signature,
                                              gostring_t data);

#elif defined(MVPN_MACOS)
#  define EXPORT __attribute__((visibility("default")))

extern "C" {
EXPORT void balrogSetLogger(logFunc func);
EXPORT unsigned char balrogValidateSignature(gostring_t publicKey,
                                             gostring_t signature,
                                             gostring_t data);
}

constexpr const char* BALROG_MACOS_UA = "Darwin_x86_64-clang-u-x86_64";

#else
#  error Platform not supported yet
#endif

constexpr const char* BALROG_CERT_SUBJECT_CN =
    "aus.content-signature.mozilla.org";

namespace {
Logger logger(LOG_NETWORKING, "Balrog");

void balrogLogger(int level, const char* msg) {
  Q_UNUSED(level);
  logger.log() << "BalrogGo:" << msg;
}

QString appVersion() {
#ifdef MVPN_INSPECTOR
  return InspectorWebSocketConnection::appVersionForUpdate();
#else
  return APP_VERSION;
#endif
}

}  // namespace

Balrog::Balrog(QObject* parent, bool downloadAndInstall)
    : Updater(parent), m_downloadAndInstall(downloadAndInstall) {
  MVPN_COUNT_CTOR(Balrog);
  logger.log() << "Balrog created";
}

Balrog::~Balrog() {
  MVPN_COUNT_DTOR(Balrog);
  logger.log() << "Balrog released";
}

// static
QString Balrog::userAgent() {
#if defined(MVPN_WINDOWS)
  static bool h =
      QSysInfo::currentCpuArchitecture().contains(QLatin1String("64"));

  return h ? BALROG_WINDOWS_UA64 : BALROG_WINDOWS_UA32;
#elif defined(MVPN_MACOS)
  return BALROG_MACOS_UA;
#else
#  error Unsupported platform
#endif
}

void Balrog::start() {
  QString url =
      QString(Constants::BALROG_URL).arg(appVersion()).arg(userAgent());
  logger.log() << "URL:" << url;

  NetworkRequest* request = NetworkRequest::createForGetUrl(this, url, 200);

  connect(request, &NetworkRequest::requestFailed,
          [this](QNetworkReply::NetworkError error, const QByteArray&) {
            logger.log() << "Request failed" << error;
            deleteLater();
          });

  connect(request, &NetworkRequest::requestCompleted,
          [this, request](const QByteArray& data) {
            logger.log() << "Request completed";

            if (!fetchSignature(request, data)) {
              logger.log() << "Ignore failure.";
              deleteLater();
            }
          });
}

bool Balrog::fetchSignature(NetworkRequest* initialRequest,
                            const QByteArray& dataUpdate) {
  Q_ASSERT(initialRequest);

  QByteArray header = initialRequest->rawHeader("Content-Signature");
  if (header.isEmpty()) {
    logger.log() << "Content-Signature missing";
    return false;
  }

  QByteArray x5u;
  QByteArray signatureBlob;
  QCryptographicHash::Algorithm algorithm = QCryptographicHash::Sha256;

  for (const QByteArray& item : header.split(';')) {
    QByteArray entry = item.trimmed();
    int pos = entry.indexOf('=');
    if (pos == -1) {
      logger.log() << "Invalid entry:" << item;
      return false;
    }

    QByteArray key = entry.left(pos);
    if (key == "x5u") {
      x5u = entry.remove(0, pos + 1);
    } else if (key == "p384ecdsa") {
      algorithm = QCryptographicHash::Sha384;
      signatureBlob = entry.remove(0, pos + 1);
    } else if (key == "p256ecdsa") {
      algorithm = QCryptographicHash::Sha256;
      signatureBlob = entry.remove(0, pos + 1);
    } else if (key == "p521ecdsa") {
      algorithm = QCryptographicHash::Sha512;
      signatureBlob = entry.remove(0, pos + 1);
    }
  }

  if (x5u.isEmpty() || signatureBlob.isEmpty()) {
    logger.log() << "No p256ecdsa/p384ecdsa or x5u found";
    return false;
  }

  logger.log() << "Fetching URL:" << x5u;

  NetworkRequest* x5uRequest = NetworkRequest::createForGetUrl(this, x5u, 200);

  connect(x5uRequest, &NetworkRequest::requestFailed,
          [this](QNetworkReply::NetworkError error, const QByteArray&) {
            logger.log() << "Request failed" << error;
            deleteLater();
          });

  connect(x5uRequest, &NetworkRequest::requestCompleted,
          [this, signatureBlob, algorithm, dataUpdate](const QByteArray& data) {
            logger.log() << "Request completed";
            if (!checkSignature(data, signatureBlob, algorithm, dataUpdate)) {
              deleteLater();
            }
          });

  return true;
}

bool Balrog::checkSignature(const QByteArray& signature,
                            const QByteArray& signatureBlob,
                            QCryptographicHash::Algorithm algorithm,
                            const QByteArray& data) {
  logger.log() << "Checking the signature";
  QList<QSslCertificate> list;
  QByteArray cert;
  for (const QByteArray& line : signature.split('\n')) {
    cert.append(line);
    cert.append('\n');

    if (line != "-----END CERTIFICATE-----") {
      continue;
    }

    QSslCertificate ssl(cert, QSsl::Pem);
    if (ssl.isNull()) {
      logger.log() << "Invalid certificate" << cert;
      return false;
    }

    list.append(ssl);
    cert.clear();
  }

  if (list.isEmpty()) {
    logger.log() << "No certificates found";
    return false;
  }

  logger.log() << "Found certificates:" << list.length();

  // TODO: do we care about OID extensions?

  // Qt5.15 doesn't implement the certificate validation (yet?)
#ifndef MVPN_MACOS
  QList<QSslError> errors = QSslCertificate::verify(list);
  for (const QSslError& error : errors) {
    if (error.error() != QSslError::SelfSignedCertificateInChain) {
      logger.log() << "Chain validation failed:" << error.errorString();
      return false;
    }
  }
#endif

  logger.log() << "Validating root certificate";
  const QSslCertificate& rootCert = list.constLast();
  QByteArray rootCertHash = rootCert.digest(QCryptographicHash::Sha256).toHex();
  if (rootCertHash != Constants::BALROG_ROOT_CERT_FINGERPRINT) {
    logger.log() << "Invalid root certificate fingerprint" << rootCertHash;
    return false;
  }

  const QSslCertificate& leaf = list.constFirst();
  logger.log() << "Validating cert subject";
  QStringList cnList = leaf.subjectInfo("CN");
  if (cnList.isEmpty() || cnList[0] != BALROG_CERT_SUBJECT_CN) {
    logger.log() << "Invalid CN:" << cnList;
    return false;
  }

  logger.log() << "Validate public key";
  QSslKey leafPublicKey = leaf.publicKey();
  if (leafPublicKey.isNull()) {
    logger.log() << "Empty public key";
    return false;
  }

  logger.log() << "Validate the signature";
  if (!validateSignature(leafPublicKey.toPem(), data, algorithm,
                         signatureBlob)) {
    logger.log() << "Invalid signature";
    return false;
  }

  logger.log() << "Fetch resource";
  if (!processData(data)) {
    logger.log() << "Fetch has failed";
    return false;
  }

  return true;
}

bool Balrog::validateSignature(const QByteArray& publicKey,
                               const QByteArray& data,
                               QCryptographicHash::Algorithm algorithm,
                               const QByteArray& signature) {
  // The algortihm is detected by the length of the signature
  Q_UNUSED(algorithm);

#if defined(MVPN_WINDOWS)
  static HMODULE balrogDll = nullptr;
  static BalrogSetLogger* balrogSetLogger = nullptr;
  static BalrogValidateSignature* balrogValidateSignature = nullptr;

  if (!balrogDll) {
    // This process will be used by the wireguard tunnel. No need to call
    // FreeLibrary.
    balrogDll = LoadLibrary(TEXT("balrog.dll"));
    if (!balrogDll) {
      WindowsCommons::windowsLog("Failed to load tunnel.dll");
      return false;
    }
  }

  if (!balrogSetLogger) {
    balrogSetLogger =
        (BalrogSetLogger*)GetProcAddress(balrogDll, "balrogSetLogger");
    if (!balrogSetLogger) {
      WindowsCommons::windowsLog("Failed to get balrogSetLogger function");
      return false;
    }
  }

  if (!balrogValidateSignature) {
    balrogValidateSignature = (BalrogValidateSignature*)GetProcAddress(
        balrogDll, "balrogValidateSignature");
    if (!balrogValidateSignature) {
      WindowsCommons::windowsLog(
          "Failed to get balrogValidateSignature function");
      return false;
    }
  }
#endif

  balrogSetLogger(balrogLogger);

  QByteArray publicKeyCopy = publicKey;
  gostring_t publicKeyGo{publicKeyCopy.constData(),
                         (size_t)publicKeyCopy.length()};

  QByteArray signatureCopy = signature;
  gostring_t signatureGo{signatureCopy.constData(),
                         (size_t)signatureCopy.length()};

  QByteArray dataCopy = data;
  gostring_t dataGo{dataCopy.constData(), (size_t)dataCopy.length()};

  unsigned char verify =
      balrogValidateSignature(publicKeyGo, signatureGo, dataGo);
  if (!verify) {
    logger.log() << "Verification failed";
    return false;
  }

  return true;
}

bool Balrog::processData(const QByteArray& data) {
  QJsonDocument json = QJsonDocument::fromJson(data);
  if (!json.isObject()) {
    logger.log() << "A valid JSON object expected";
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
      logger.log() << "Invalid URL in the JSON document";
      return false;
    }

    NetworkRequest* request = NetworkRequest::createForGetUrl(this, url);

    connect(request, &NetworkRequest::requestFailed,
            [this](QNetworkReply::NetworkError error, const QByteArray&) {
              logger.log() << "Request failed" << error;
              deleteLater();
            });

    connect(request, &NetworkRequest::requestHeaderReceived,
            [this](NetworkRequest* request) {
              Q_ASSERT(request);
              logger.log() << "Request header received";

              // We want to proceed only if the status code is 200. The request
              // will be aborted, but the signal emitted.
              if (request->statusCode() == 200) {
                emit updateRecommended();
              }

              logger.log() << "Abort request for status code"
                           << request->statusCode();
              request->abort();
            });

    connect(request, &NetworkRequest::requestCompleted,
            [this](const QByteArray&) {
              logger.log() << "Request completed";
              deleteLater();
            });

    return true;
  }

  QString url = obj.value("url").toString();
  if (url.isEmpty()) {
    logger.log() << "Invalid URL in the JSON document";
    return false;
  }

  QString hashFunction = obj.value("hashFunction").toString();
  if (hashFunction.isEmpty()) {
    logger.log() << "No hashFunction item";
    return false;
  }

  QString hashValue = obj.value("hashValue").toString();
  if (hashValue.isEmpty()) {
    logger.log() << "No hashValue item";
    return false;
  }

  NetworkRequest* request = NetworkRequest::createForGetUrl(this, url);

  // No timeout for this request.
  request->disableTimeout();

  connect(
      request, &NetworkRequest::requestFailed,
      [this, request](QNetworkReply::NetworkError error, const QByteArray&) {
        logger.log() << "Request failed" << error;
        propagateError(request, error);
        deleteLater();
      });

  connect(request, &NetworkRequest::requestCompleted,
          [this, hashValue, hashFunction, url](const QByteArray& data) {
            logger.log() << "Request completed";

            if (!computeHash(url, data, hashValue, hashFunction)) {
              logger.log() << "Ignore failure.";
              deleteLater();
            }
          });

  return true;
}

bool Balrog::computeHash(const QString& url, const QByteArray& data,
                         const QString& hashValue,
                         const QString& hashFunction) {
  logger.log() << "Compute the hash";

  if (hashFunction != "sha512") {
    logger.log() << "Invalid hash function";
    return false;
  }

  QCryptographicHash hash(QCryptographicHash::Sha512);
  hash.addData(data);
  QByteArray hashHex = hash.result().toHex();

  if (hashHex != hashValue) {
    logger.log() << "Hash doesn't match";
    return false;
  }

  return saveFileAndInstall(url, data);
}

bool Balrog::saveFileAndInstall(const QString& url, const QByteArray& data) {
  logger.log() << "Savel the file and install it";

  int pos = url.lastIndexOf("/");
  if (pos == -1) {
    logger.log() << "The URL seems to be without /.";
    return false;
  }

  QString fileName = url.right(url.length() - pos - 1);
  logger.log() << "Filename:" << fileName;

  if (!m_tmpDir.isValid()) {
    logger.log() << "Cannot create a temporary directory";
    return false;
  }

  QDir dir(m_tmpDir.path());
  QString tmpFile = dir.filePath(fileName);

  QFile file(tmpFile);
  if (!file.open(QIODevice::ReadWrite)) {
    logger.log() << "Unable to create a file in the temporary folder";
    return false;
  }

  qint64 written = file.write(data);
  if (written != data.length()) {
    logger.log() << "Unable to write the whole configuration file";
    return false;
  }

  file.close();

  return install(tmpFile);
}

bool Balrog::install(const QString& filePath) {
  logger.log() << "Install the package:" << filePath;

  QString logFile = m_tmpDir.filePath("msiexec.log");

#if defined(MVPN_WINDOWS)
  QStringList arguments;
  arguments << "/qb!-"
            << "REBOOT=ReallySuppress"
            << "/i" << QDir::toNativeSeparators(filePath) << "/lv"
            << QDir::toNativeSeparators(logFile);

  QProcess* process = new QProcess(this);
  process->start("msiexec.exe", arguments);
  connect(process,
          QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
          [this, process, logFile](int exitCode, QProcess::ExitStatus) {
            logger.log() << "Installation completed - exitCode:" << exitCode;

            logger.log() << "Stdout:" << Qt::endl
                         << qUtf8Printable(process->readAllStandardOutput())
                         << Qt::endl;
            logger.log() << "Stderr:" << Qt::endl
                         << qUtf8Printable(process->readAllStandardError())
                         << Qt::endl;

            QFile log(logFile);
            if (!log.open(QIODevice::ReadOnly | QIODevice::Text)) {
              logger.log() << "Unable to read the msiexec log file";
            } else {
              logger.log() << "Log file:" << Qt::endl << log.readAll();
            }

            if (exitCode != 0) {
              deleteLater();
              return;
            }

            // We leak the object because the installer will restart the
            // app and we need to keep the temporary folder alive during the
            // whole process.
          });
#endif

#if defined(MVPN_MACOS)
  QStringList arguments;
  arguments << filePath;

  QProcess* process = new QProcess(this);
  process->start("open", arguments);
  connect(process,
          QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
          [this, process](int exitCode, QProcess::ExitStatus) {
            logger.log() << "Installation completed - exitCode:" << exitCode;

            logger.log() << "Stdout:" << Qt::endl
                         << qUtf8Printable(process->readAllStandardOutput())
                         << Qt::endl;
            logger.log() << "Stderr:" << Qt::endl
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

  return true;
}

void Balrog::propagateError(NetworkRequest* request,
                            QNetworkReply::NetworkError error) {
  Q_ASSERT(request);

  MozillaVPN* vpn = MozillaVPN::instance();
  Q_ASSERT(vpn);

  // 451 Unavailable For Legal Reasons
  if (request->statusCode() == 451) {
    logger.log() << "Geo IP restriction detected";
    vpn->errorHandle(ErrorHandler::GeoIpRestrictionError);
    return;
  }

  vpn->errorHandle(ErrorHandler::toErrorType(error));
}
