/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "balrog.h"
#include "leakdetector.h"
#include "logger.h"
#include "networkrequest.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMessageAuthenticationCode>
#include <QScopeGuard>
#include <QSslCertificate>
#include <QSslKey>
#include <QTemporaryDir>

#include <openssl/bio.h>
#include <openssl/ec.h>
#include <openssl/err.h>
#include <openssl/ossl_typ.h>
#include <openssl/pem.h>

constexpr const char* BALROG_URL =
    "https://aus5.mozilla.org/json/1/FirefoxVPN/%1/%2/release/update.json";

constexpr const char* BALROG_WINDOWS_UA64 = "WINNT_x86_64";
constexpr const char* BALROG_WINDOWS_UA32 = "WINNT_x86_32";

constexpr const char* BALROG_ROOT_CERT_FINGERPRINT =
    "97e8ba9cf12fb3de53cc42a4e6577ed64df493c247b414fea036818d3823560e";
constexpr const char* BALROG_CERT_SUBJECT_CN =
    "aus.content-signature.mozilla.org";

namespace {
Logger logger(LOG_NETWORKING, "Balrog");
}

Balrog::Balrog(QObject* parent) : Updater(parent) {
  MVPN_COUNT_CTOR(Balrog);
  logger.log() << "Balrog created";
}

Balrog::~Balrog() {
  MVPN_COUNT_DTOR(Balrog);
  logger.log() << "Balrog released";
}

// static
QString Balrog::userAgent() {
  static bool h =
      QSysInfo::currentCpuArchitecture().contains(QLatin1String("64"));
  return h ? BALROG_WINDOWS_UA64 : BALROG_WINDOWS_UA32;
}

void Balrog::start() {
  QString url =
      QString(BALROG_URL).arg(/* TODO APP_VERSION */ "1").arg(userAgent());
  logger.log() << "URL:" << url;

  NetworkRequest* request = NetworkRequest::createForUrl(this, url);

  connect(request, &NetworkRequest::requestFailed,
          [this](QNetworkReply::NetworkError error, const QByteArray&) {
            logger.log() << "Request failed" << error;
            deleteLater();
          });

  connect(request, &NetworkRequest::requestCompleted,
          [this](QNetworkReply* reply, const QByteArray& data) {
            logger.log() << "Request completed";

            if (!fetchSignature(reply, data)) {
              logger.log() << "Ignore failure.";
              deleteLater();
            }
          });
}

bool Balrog::processData(const QByteArray& data) {
  QJsonDocument json = QJsonDocument::fromJson(data);
  if (!json.isObject()) {
    logger.log() << "A valid JSON object expected";
    return false;
  }

  QJsonObject obj = json.object();

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

  NetworkRequest* request = NetworkRequest::createForUrl(this, url);

  connect(request, &NetworkRequest::requestFailed,
          [this](QNetworkReply::NetworkError error, const QByteArray&) {
            logger.log() << "Request failed" << error;
            deleteLater();
          });

  connect(request, &NetworkRequest::requestCompleted,
          [this, hashValue, hashFunction, url](QNetworkReply*,
                                               const QByteArray& data) {
            logger.log() << "Request completed";

            if (!computeHash(url, data, hashValue, hashFunction)) {
              logger.log() << "Ignore failure.";
              deleteLater();
            }
          });

  return true;
}

bool Balrog::fetchSignature(QNetworkReply* reply,
                            const QByteArray& dataUpdate) {
  Q_ASSERT(reply);

  QByteArray header = reply->rawHeader("Content-Signature");
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

  signatureBlob =
      QByteArray::fromBase64(signatureBlob, QByteArray::Base64UrlEncoding);
  if (signatureBlob.isEmpty()) {
    logger.log() << "Invalid p256ecdsa/p384ecdsa entry";
    return false;
  }

  logger.log() << "Fetching URL:" << x5u;

  NetworkRequest* request = NetworkRequest::createForUrl(this, x5u);

  connect(request, &NetworkRequest::requestFailed,
          [this](QNetworkReply::NetworkError error, const QByteArray&) {
            logger.log() << "Request failed" << error;
            deleteLater();
          });

  connect(request, &NetworkRequest::requestCompleted,
          [this, signatureBlob, algorithm, dataUpdate](QNetworkReply*,
                                                       const QByteArray& data) {
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
  for (const QByteArray line : signature.split('\n')) {
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

  QList<QSslError> errors = QSslCertificate::verify(list);
  for (const QSslError error : errors) {
    if (error.error() != QSslError::SelfSignedCertificateInChain) {
      logger.log() << "Chain validation failed:" << error.errorString();
      return false;
    }
  }

  logger.log() << "Validating root certificate";
  const QSslCertificate& rootCert = list.constLast();
  QByteArray rootCertHash = rootCert.digest(QCryptographicHash::Sha256).toHex();
  if (rootCertHash != BALROG_ROOT_CERT_FINGERPRINT) {
    logger.log() << "Invalid root certificate fingerprint";
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
  BIO* bio = nullptr;
  EC_KEY* key = nullptr;
  ECDSA_SIG* sig = nullptr;

  auto cleanup = qScopeGuard([&] {
    if (bio) BIO_free(bio);
    if (key) EC_KEY_free(key);
    if (sig) ECDSA_SIG_free(sig);
  });

  bio = BIO_new(BIO_s_mem());
  if (!bio) {
    logger.log() << "Failed to create a BIO object";
    return false;
  }

  if (BIO_write(bio, publicKey.constData(), publicKey.length()) !=
      publicKey.length()) {
    logger.log() << "Failed to write the public key";
    return false;
  }

  int nid = 0;
  switch (algorithm) {
    case QCryptographicHash::Sha384:
      nid = NID_secp384r1;
      break;
    case QCryptographicHash::Sha256:
      nid = NID_X9_62_prime256v1;
      break;
    case QCryptographicHash::Sha512:
      nid = NID_secp521r1;
      break;
    default:
      qFatal("Invalid hash");
  }

  key = EC_KEY_new_by_curve_name(nid);
  if (!key) {
    logger.log() << "Failed to create the key object";
    return false;
  }

  if (!PEM_read_bio_EC_PUBKEY(bio, &key, nullptr, nullptr)) {
    logger.log() << "Failed to create the key";
    return false;
  }

  if (!EC_KEY_check_key(key)) {
    logger.log() << "Invalid key";
    return false;
  }

  const unsigned char* signaturePtr =
      (const unsigned char*)signature.constData();

  BIGNUM* sig_r = BN_new();
  BIGNUM* sig_s = BN_new();

  uint32_t rs_size = signature.length() / 2;

  if (!BN_bin2bn(&signaturePtr[0], rs_size, sig_r)) {
    logger.log() << "Failed to set the R value";
    return false;
  }

  if (!BN_bin2bn(&signaturePtr[rs_size], rs_size, sig_s)) {
    logger.log() << "Failed to set the R value";
    return false;
  }

  logger.log() << "R:" << BN_bn2hex(sig_r);
  logger.log() << "S:" << BN_bn2hex(sig_s);

  sig = ECDSA_SIG_new();
  ECDSA_SIG_set0(sig, sig_r, sig_s);

  QByteArray digest;
  digest.append("Content-Signature:");
  digest.append("\0");
  digest.append(data);

  if (ECDSA_do_verify((const unsigned char*)digest.constData(), digest.length(),
                      sig, key) == 0) {
    logger.log() << "Verification error:"
                 << ERR_error_string(ERR_get_error(), nullptr);
    return /* TODO: false */ true;
  }

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

  QTemporaryDir tmpDir;
  if (!tmpDir.isValid()) {
    logger.log() << "Cannot create a temporary directory";
    return false;
  }

  QDir dir(tmpDir.path());
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
  logger.log() << "Install the pacakge:" << filePath;
  // TODO
  deleteLater();
  return true;
}
