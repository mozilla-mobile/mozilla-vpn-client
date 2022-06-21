/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef GUIDEMODEL_H
#define GUIDEMODEL_H

#include <QList>
#include <QAbstractListModel>

class Guide;
class QJsonObject;

class GuideModel final : public QAbstractListModel {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(GuideModel)

 public:
  enum ModelRoles {
    GuideRole = Qt::UserRole + 1,
  };

  static GuideModel* instance();

  ~GuideModel();

  QStringList guideTitleIds() const;

  bool createFromJson(const QString& addonId, const QJsonObject& obj);

  void remove(const QString& addonId);

  // QAbstractListModel methods

  QHash<int, QByteArray> roleNames() const override;

  int rowCount(const QModelIndex&) const override;

  QVariant data(const QModelIndex& index, int role) const override;

 private:
  explicit GuideModel(QObject* parent);

  struct GuideData {
    QString m_addonId;
    Guide* m_guide;
  };
  QList<GuideData> m_guides;
};

#endif  // GUIDEMODEL_H
