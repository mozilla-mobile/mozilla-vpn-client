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
}

QAbstractListModel* FilterProxyModel::source() const {
  return qobject_cast<QAbstractListModel*>(sourceModel());
}

void FilterProxyModel::setSource(QAbstractListModel* sourceModel) {
  setSourceModel(sourceModel);

  if (sourceModel) {
    m_sourceModelRoleNames = sourceModel->roleNames();
  } else {
    m_sourceModelRoleNames.clear();
  }
}

bool FilterProxyModel::filterAcceptsRow(
    int source_row, const QModelIndex& source_parent) const {
  if (m_filterCallback.isNull()) {
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

  QJSEngine* engine = QmlEngineHolder::instance()->engine();
  Q_ASSERT(engine);

  QJSValue value = engine->newObject();
  Q_ASSERT(value.isObject());

  for (QHash<int, QByteArray>::const_iterator i =
           m_sourceModelRoleNames.constBegin();
       i != m_sourceModelRoleNames.constEnd(); ++i) {
    QVariant data = sourceModel()->data(index, i.key());
    value.setProperty(QString(i.value()), engine->toScriptValue(data));
  }

  QJSValueList arguments;
  arguments.append(value);

  QJSValue retValue = m_filterCallback.call(arguments);
  return retValue.toBool();
}
