/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TUTORIALMODEL_H
#define TUTORIALMODEL_H

#include <QList>
#include <QAbstractListModel>

class Tutorial;

class TutorialModel final : public QAbstractListModel {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(TutorialModel)

 public:
  enum ModelRoles {
    TutorialRole = Qt::UserRole + 1,
  };

  static TutorialModel* instance();

  ~TutorialModel();

  // QAbstractListModel methods

  QHash<int, QByteArray> roleNames() const override;

  int rowCount(const QModelIndex&) const override;

  QVariant data(const QModelIndex& index, int role) const override;

 private:
  explicit TutorialModel(QObject* parent);

  void initialize();

  QList<Tutorial*> m_tutorials;
};

#endif  // TUTORIALMODEL_H
