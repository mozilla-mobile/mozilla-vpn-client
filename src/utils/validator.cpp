/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "validator.h"

#include <QCoreApplication>
#include <QFile>
#include <QMetaEnum>
#include <QRegularExpression>

#include "leakdetector.h"
#include "logger.h"

namespace {
Logger logger("Validator");
Validator* s_instance = nullptr;
}  // namespace

// static
Validator* Validator::instance() {
  if (!s_instance) {
    s_instance = new Validator(qApp);
    qAddPostRoutine([]() { delete s_instance; });
  }
  return s_instance;
}

Validator::Validator(QObject* parent) : QObject(parent) {
  MZ_COUNT_CTOR(Validator);
  Q_ASSERT(!s_instance);
}

Validator::~Validator() {
  MZ_COUNT_DTOR(Validator);
  Q_ASSERT(s_instance == this);
  s_instance = nullptr;
}

// static
bool Validator::validateEmailAddress(const QString& emailAddress) {
  // https://github.com/mozilla/fxa/blob/main/packages/fxa-auth-server/lib/routes/validators.js#L144-L177

  if (emailAddress.isEmpty()) {
    return false;
  }

  QStringList parts = emailAddress.split("@");
  if (parts.length() != 2 || parts[1].length() > 255) {
    return false;
  }

  static QRegularExpression emailRE("^[A-Z0-9.!#$%&'*+/=?^_`{|}~-]{1,64}$",
                                    QRegularExpression::CaseInsensitiveOption);
  // We don't have to convert the first part of the email address to ASCII
  // Compatible Encoding (ace).
  if (!emailRE.match(parts[0]).hasMatch()) {
    return false;
  }

  QByteArray domainAce = QUrl::toAce(parts[1]);
  static QRegularExpression domainRE(
      "^[A-Z0-9](?:[A-Z0-9-]{0,253}[A-Z0-9])?(?:\\.[A-Z0-9](?:[A-Z0-9-]{0,253}["
      "A-Z0-9])?)+$",
      QRegularExpression::CaseInsensitiveOption);
  if (!domainRE.match(domainAce).hasMatch()) {
    return false;
  }

  return true;
}