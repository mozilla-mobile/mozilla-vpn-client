/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "qmlpath.h"

#include <QQmlApplicationEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include <QRegularExpression>
#include <QTest>

QmlPath::QmlPath(const QString& path) {
  if (path.isEmpty()) {
    return;
  }

  qsizetype size = path.size();
  const QChar* input = path.constData();

  QList<Data> blocks;

  while (true) {
    if (!parsePath(input, size, blocks)) {
      return;
    }

    if (size == 0 || *input != '/') {
      break;
    }
  }

  if (size != 0) {
    return;
  }

  m_blocks.swap(blocks);
}

// static
bool QmlPath::parsePath(const QChar*& input, qsizetype& size,
                        QList<Data>& blocks) {
  Q_ASSERT(input);
  Q_ASSERT(size);

  Data data;
  if (*input == '/') {
    ++input;
    if (!--size) {
      blocks.append(data);
      return true;
    }
  }

  if (*input == '/') {
    data.m_nested = true;
    ++input;
    if (!--size) return false;
  }

  while (size && *input != '/' && *input != '[' && *input != '{') {
    data.m_key.append(*input);
    ++input;
    --size;
  }

  while (size && *input != '/') {
    if (*input == '[') {
      ++input;
      --size;

      QString number;
      while (size && *input != ']') {
        number.append(*input);
        ++input;
        --size;
      }

      if (size == 0 || *input != ']') return false;

      ++input;
      --size;

      Filter filter;
      filter.m_type = Filter::Index;

      bool ok = false;
      filter.m_index = number.toInt(&ok);
      if (!ok || filter.m_index < 0) return false;

      data.m_filters.append(filter);
      continue;
    }

    if (*input == '{') {
      ++input;
      --size;

      QString propertyValue;
      while (size && *input != '}') {
        propertyValue.append(*input);
        ++input;
        --size;
      }

      if (size == 0 || *input != '}') return false;

      ++input;
      --size;

      QStringList parts = propertyValue.split('=');
      if (parts.length() > 2) return false;

      Filter filter;
      filter.m_type = Filter::Property;

      filter.m_property = parts[0].toLocal8Bit();
      if (filter.m_property.isEmpty()) return false;

      if (parts.length() > 1) {
        filter.m_hasPropertyValue = true;
        filter.m_propertyValue = parts[1];
      }

      data.m_filters.append(filter);
      continue;
    }

    return false;
  }

  blocks.append(data);
  return true;
}

QQuickItem* QmlPath::evaluate(QQmlApplicationEngine* engine) const {
  if (!engine) {
    return nullptr;
  }

  if (m_blocks.isEmpty()) {
    return nullptr;
  }

  QList<QQuickItem*> list;
  for (QObject* object : engine->rootObjects()) {
    QQuickItem* item = qobject_cast<QQuickItem*>(object);
    if (item) {
      list.append(item);
    } else {
      for (QObject* child : object->children()) {
        QQuickItem* item = qobject_cast<QQuickItem*>(child);
        if (item) {
          list.append(item);
        }
      }
    }
  }

  return evaluateItems(nullptr, list, m_blocks.cbegin());
}

QQuickItem* QmlPath::evaluateItems(QQuickItem* currentItem,
                                   const QList<QQuickItem*>& items,
                                   QList<Data>::const_iterator i) const {
  if (i == m_blocks.cend()) {
    return currentItem;
  }

  if (items.isEmpty()) {
    return nullptr;
  }

  QList<QQuickItem*> results;

  if (i->m_key.isEmpty()) {
    results.append(items);
  } else {
    for (QQuickItem* item : items) {
      if (item->objectName() == i->m_key) {
        results.append(item);
      }
    }

    if (i->m_nested) {
      for (QQuickItem* item : items) {
        results.append(findItems(item, i->m_key));
      }
    }
  }

  for (const Filter& filter : i->m_filters) {
    switch (filter.m_type) {
      case Filter::Index:
        results = filterByIndex(results, filter);
        break;

      case Filter::Property:
        filterByProperty(results, filter);
        break;
    }
  }

  for (QQuickItem* result : results) {
    QQuickItem* item = evaluateItems(result, collectChildItems(result), i + 1);
    if (item) return item;
  }

  return nullptr;
}

// static
QList<QQuickItem*> QmlPath::collectChildItems(QQuickItem* item) {
  Q_ASSERT(item);
  QList<QQuickItem*> list = item->childItems();

  auto* contentItem = item->property("contentItem").value<QQuickItem*>();
  if (contentItem) {
    list.append(collectChildItems(contentItem));
  }

  return list;
}

// static
QList<QQuickItem*> QmlPath::findItems(QQuickItem* item, const QString& key) {
  QList<QQuickItem*> list;

  for (QQuickItem* child : collectChildItems(item)) {
    if (child->objectName() == key) {
      list.append(child);
    }
    list.append(findItems(child, key));
  }

  return list;
}

// static
QList<QQuickItem*> QmlPath::filterByIndex(const QList<QQuickItem*>& items,
                                          const Filter& filter) {
  QList<QQuickItem*> results;
  if (filter.m_index < items.length()) {
    results.append(items[filter.m_index]);
  }
  return results;
}

// static
void QmlPath::filterByProperty(QList<QQuickItem*>& items,
                               const Filter& filter) {
  QMutableListIterator<QQuickItem*> i(items);
  while (i.hasNext()) {
    QQuickItem* item = i.next();

    // Special class name magic
    if (filter.m_property == "className" && filter.m_hasPropertyValue) {
      QString className(item->metaObject()->className());
      if (className.contains("QQuick")) {
        static QRegularExpression classNameRE("_QML_[0-9]*.*");
        className.remove(classNameRE);
        className.remove("QQuick");
      } else if (className.contains("QMLTYPE")) {
        static QRegularExpression classNameRE("_QMLTYPE_[0-9]*.*");
        className.remove(classNameRE);
      }

      if (className != filter.m_propertyValue) {
        i.remove();
      }

      continue;
    }

    QVariant property = item->property(filter.m_property.data());
    if (!property.isValid()) {
      i.remove();
      continue;
    }

    if (!filter.m_hasPropertyValue) continue;

    if (property.toString() != filter.m_propertyValue) {
      i.remove();
    }
  }
}
