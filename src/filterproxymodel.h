/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FILTERPROXYMODEL_H
#define FILTERPROXYMODEL_H

#include <QQmlEngine>
#include <QHash>
#include <QJSValue>
#include <QQmlParserStatus>
#include <QSortFilterProxyModel>

class FilterProxyModel : public QSortFilterProxyModel, public QQmlParserStatus {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(FilterProxyModel)
  Q_INTERFACES(QQmlParserStatus)

  QML_NAMED_ELEMENT(VPNFilterProxyModel)

  Q_PROPERTY(QJSValue filterCallback READ filterCallback WRITE setFilterCallback
                 NOTIFY filterCallbackChanged)
  Q_PROPERTY(QJSValue sortCallback READ sortCallback WRITE setSortCallback
                 NOTIFY sortCallbackChanged)
  Q_PROPERTY(QAbstractListModel* source READ source WRITE setSource)
  Q_PROPERTY(int count READ count NOTIFY countChanged)

 public:
  FilterProxyModel(QObject* parent = 0);

  virtual ~FilterProxyModel() = default;

  Q_INVOKABLE QVariant get(int pos) const;

 signals:
  void filterCallbackChanged();
  void sortCallbackChanged();
  void countChanged();

 public:
  QJSValue filterCallback() const;
  void setFilterCallback(QJSValue filterCallback);

  QJSValue sortCallback() const;
  void setSortCallback(QJSValue sortCallback);

  QAbstractListModel* source() const;
  void setSource(QAbstractListModel* sourceModel);

  QJSValue dataToJSValue(const QAbstractItemModel* model,
                         const QModelIndex& index) const;

  // QSortFilterProxyModel methods

  bool filterAcceptsRow(int source_row,
                        const QModelIndex& source_parent) const override;

  bool lessThan(const QModelIndex& left,
                const QModelIndex& right) const override;

  // QQmlParserStatus

  void classBegin() override;
  void componentComplete() override;

  int count() const { return rowCount(); }

 private:
  mutable QJSValue m_filterCallback;
  mutable QJSValue m_sortCallback;

  QHash<int, QByteArray> m_sourceModelRoleNames;

  bool m_completed = false;
};

#endif  // FILTERPROXYMODEL_H
