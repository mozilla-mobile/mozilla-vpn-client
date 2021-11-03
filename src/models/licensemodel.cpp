/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "licensemodel.h"
#include "leakdetector.h"
#include "logger.h"

#include <QFile>

namespace {
Logger logger(LOG_MODEL, "LicenseModel");
}

LicenseModel::LicenseModel() { MVPN_COUNT_CTOR(LicenseModel); }

LicenseModel::~LicenseModel() { MVPN_COUNT_DTOR(LicenseModel); }

QHash<int, QByteArray> LicenseModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[TitleRole] = "licenseTitle";
  roles[ContentRole] = "licenseContent";
  return roles;
}

int LicenseModel::rowCount(const QModelIndex&) const {
  // The first license is shown differently.
  return m_licenses.isEmpty() ? 0 : m_licenses.count() - 1;
}

QVariant LicenseModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }

  // The +1 is because the first license is shown differently.

  switch (role) {
    case TitleRole:
      return QVariant(m_licenses.at(index.row() + 1).m_title);

    case ContentRole:
      return QVariant(m_licenses.at(index.row() + 1).m_content);

    default:
      return QVariant();
  }
}

void LicenseModel::initialize() {
  if (!m_licenses.isEmpty()) return;

  QFile file(":/license/LICENSE.md");
  if (!file.open(QFile::ReadOnly | QFile::Text)) {
    logger.error() << "Failed to open the license file";
    return;
  }

  QList<QByteArray> content = file.readAll().split('\n');

  beginResetModel();

  for (uint32_t i = 0, length = content.length(); i < length; ++i) {
    int32_t lineLength = content[i].length();
    if (i < length - 1 && lineLength > 0 &&
        lineLength == content[i + 1].length() &&
        content[i + 1].startsWith("===")) {
      m_licenses.append(License{content[i], ""});
      ++i;
      continue;
    }

    Q_ASSERT(!m_licenses.isEmpty());
    m_licenses.last().m_content += content[i] + "\n";
  }

  endResetModel();

  emit contentLicenseChanged();
}

QString LicenseModel::contentLicense() const {
  return m_licenses.isEmpty() ? QString() : m_licenses[0].m_content;
}
