/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FILTERPROXYMODEL_H
#define FILTERPROXYMODEL_H

#include <QQmlEngine>
#include <QHash>
#include <QJSValue>
#include <QSortFilterProxyModel>

class FilterProxyModel : public QSortFilterProxyModel {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(FilterProxyModel)
  QML_NAMED_ELEMENT(VPNFilterProxyModel)

  Q_PROPERTY(
      QJSValue filterCallback READ filterCallback WRITE setFilterCallback)
  Q_PROPERTY(QAbstractListModel* source READ source WRITE setSource)

 public:
  FilterProxyModel(QObject* parent = 0);

  virtual ~FilterProxyModel() = default;

 public:
  QJSValue filterCallback() const;
  void setFilterCallback(QJSValue filterCallback);

  QAbstractListModel* source() const;
  void setSource(QAbstractListModel* sourceModel);

  // QSortFilterProxyModel methods

  bool filterAcceptsRow(int source_row,
                        const QModelIndex& source_parent) const override;

 private:
  mutable QJSValue m_filterCallback;

  QHash<int, QByteArray> m_sourceModelRoleNames;
};

#endif  // FILTERPROXYMODEL_H
