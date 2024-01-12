/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "inspectorutils.h"

#include <QQmlApplicationEngine>
#include <QQuickItem>

#include "context/qmlengineholder.h"
#include "utilities/qmlpath.h"

// static
QObject* InspectorUtils::findObject(const QString& name) {
  QStringList parts = name.split("/");
  Q_ASSERT(!parts.isEmpty());

  QQuickItem* parent = nullptr;
  QQmlApplicationEngine* engine = qobject_cast<QQmlApplicationEngine*>(
      QmlEngineHolder::instance()->engine());
  if (!engine) {
    return nullptr;
  }

  for (QObject* rootObject : engine->rootObjects()) {
    if (!rootObject) {
      continue;
    }

    parent = rootObject->findChild<QQuickItem*>(parts[0]);
    if (parent) {
      break;
    }
  }

  if (!parent) {
    if (parts.length() == 1) {
      int id = qmlTypeId("Mozilla.VPN", 1, 0, qPrintable(parts[0]));
      return engine->singletonInstance<QObject*>(id);
    }
    return parent;
  }

  for (int i = 1; i < parts.length(); ++i) {
    QList<QQuickItem*> children = parent->childItems();

    bool found = false;
    for (QQuickItem* item : children) {
      if (item->objectName() == parts[i]) {
        parent = item;
        found = true;
        break;
      }
    }

    if (!found) {
      QQuickItem* contentItem =
          parent->property("contentItem").value<QQuickItem*>();
      if (!contentItem) {
        return nullptr;
      }

      QList<QQuickItem*> contentItemChildren = contentItem->childItems();

      for (QQuickItem* item : contentItemChildren) {
        if (item->objectName() == parts[i]) {
          parent = item;
          found = true;
          break;
        }
      }
    }

    if (!found) {
      return nullptr;
    }
  }

  return parent;
}

QObject* InspectorUtils::queryObject(const QString& path) {
  QmlPath qmlPath(path);
  if (!qmlPath.isValid()) {
    return nullptr;
  }

  QQmlApplicationEngine* engine = qobject_cast<QQmlApplicationEngine*>(
      QmlEngineHolder::instance()->engine());
  if (!engine) {
    return nullptr;
  }

  return qmlPath.evaluate(engine);
}
