/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "filterproxymodel.h"
#include "logger.h"
#include "qmlengineholder.h"

namespace {
Logger logger(LOG_MODEL, "FilterProxyModel");
}

FilterProxyModel::FilterProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent) {}

QJSValue FilterProxyModel::filterCallback() const { return m_filterCallback; }

void FilterProxyModel::setFilterCallback(QJSValue filterCallback) {
  if (!filterCallback.isCallable()) {
    logger.error()
        << "FilterProxyModel.filterCallback must be a JS callable value";
    return;
  }

  m_filterCallback = filterCallback;
  invalidate();

  emit filterCallbackChanged();
}

QJSValue FilterProxyModel::sortCallback() const { return m_sortCallback; }

void FilterProxyModel::setSortCallback(QJSValue sortCallback) {
  if (!sortCallback.isCallable()) {
    logger.error()
        << "FilterProxyModel.sortCallback must be a JS callable value";
    return;
  }

  m_sortCallback = sortCallback;
  invalidate();

  emit sortCallbackChanged();
}

QAbstractListModel* FilterProxyModel::source() const {
  return qobject_cast<QAbstractListModel*>(sourceModel());
}

void FilterProxyModel::setSource(QAbstractListModel* sourceModel) {
  setSourceModel(sourceModel);

  if (sourceModel) {
    connect(sourceModel, &QAbstractItemModel::rowsInserted, this,
            &FilterProxyModel::countChanged);
    connect(sourceModel, &QAbstractItemModel::rowsRemoved, this,
            &FilterProxyModel::countChanged);
    connect(sourceModel, &QAbstractItemModel::modelReset, this,
            &FilterProxyModel::countChanged);
    connect(sourceModel, &QAbstractItemModel::layoutChanged, this,
            &FilterProxyModel::countChanged);
    m_sourceModelRoleNames = sourceModel->roleNames();
  } else {
    m_sourceModelRoleNames.clear();
  }
}

QVariant FilterProxyModel::get(int pos) const {
  QModelIndex i = index(pos, 0);
  QJSValue value = dataToJSValue(this, i);
  return QVariant::fromValue(value);
}

bool FilterProxyModel::filterAcceptsRow(
    int source_row, const QModelIndex& source_parent) const {
  if (!m_completed) {
    return false;
  }

  if (m_filterCallback.isNull() || m_filterCallback.isUndefined()) {
    logger.debug() << "No filter callback set!";
    return true;
  }

  Q_ASSERT(m_filterCallback.isCallable());

  auto index = sourceModel()->index(source_row, 0, source_parent);
  if (!index.isValid()) {
    return false;
  }

  if (!QmlEngineHolder::exists()) {
    logger.error() << "Something bad is happening. Are we shutting down?";
    return false;
  }

  QJSValue value = dataToJSValue(sourceModel(), index);

  QJSValueList arguments;
  arguments.append(value);

  QJSValue retValue = m_filterCallback.call(arguments);
  if (retValue.isError()) {
    logger.debug() << "Execution throws an error:" << retValue.toString();
    return false;
  }

  return retValue.toBool();
}

bool FilterProxyModel::lessThan(const QModelIndex& left,
                                const QModelIndex& right) const {
  if (!m_completed) {
    return false;
  }

  if (m_sortCallback.isNull() || m_sortCallback.isUndefined()) {
    return QSortFilterProxyModel::lessThan(left, right);
  }

  Q_ASSERT(m_sortCallback.isCallable());

  if (!QmlEngineHolder::exists()) {
    logger.error() << "Something bad is happening. Are we shutting down?";
    return false;
  }

  QJSValue valueA = dataToJSValue(sourceModel(), left);
  QJSValue valueB = dataToJSValue(sourceModel(), right);

  QJSValueList arguments;
  arguments.append(valueA);
  arguments.append(valueB);

  QJSValue retValue = m_sortCallback.call(arguments);
  if (retValue.isError()) {
    logger.debug() << "Execution throws an error:" << retValue.toString();
    return false;
  }

  return retValue.toBool();
}

void FilterProxyModel::classBegin() {}

void FilterProxyModel::componentComplete() {
  m_completed = true;
  invalidate();

  if (m_sortCallback.isCallable()) {
    sort(0);
  }
}

QJSValue FilterProxyModel::dataToJSValue(const QAbstractItemModel* model,
                                         const QModelIndex& index) const {
  QJSEngine* engine = QmlEngineHolder::instance()->engine();
  Q_ASSERT(engine);

  QJSValue value = engine->newObject();
  Q_ASSERT(value.isObject());

  for (QHash<int, QByteArray>::const_iterator i =
           m_sourceModelRoleNames.constBegin();
       i != m_sourceModelRoleNames.constEnd(); ++i) {
    QVariant data = model->data(index, i.key());
    value.setProperty(QString(i.value()), engine->toScriptValue(data));
  }

  return value;
}
