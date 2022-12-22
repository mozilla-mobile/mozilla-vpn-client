/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LICENSEMODEL_H
#define LICENSEMODEL_H

#include <QAbstractListModel>

class LicenseModel final : public QAbstractListModel {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(LicenseModel)

  Q_PROPERTY(
      QString contentLicense READ contentLicense NOTIFY contentLicenseChanged)

 public:
  static LicenseModel* instance();

  ~LicenseModel();

  enum ModelRoles {
    TitleRole = Qt::UserRole + 1,
    ContentRole,
  };

  // We don't want to load the licenses if not needed.
  Q_INVOKABLE void initialize();

  QString contentLicense() const;

  // QAbstractListModel methods

  QHash<int, QByteArray> roleNames() const override;

  int rowCount(const QModelIndex&) const override;

  QVariant data(const QModelIndex& index, int role) const override;

 signals:
  void contentLicenseChanged();

 private:
  explicit LicenseModel(QObject* parent);

 private:
  struct License {
    QString m_title;
    QString m_content;
  };

  QList<License> m_licenses;
};

#endif  // LICENSEMODEL_H
