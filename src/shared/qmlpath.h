/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QObject>

class QQmlApplicationEngine;
class QQuickItem;

/**
 * @brief Filter a QML tree using an XPath-like syntax
 *
 * The syntax is composed of blocks of "path". A path can be one of the
 * following:
 *
 * - '/' -> all the items at the current QML tree level
 * - '/' + objectName - the item with the name `objectName` as a child of the
 *   current QML tree item
 * - '//' + objectName - search of an item with the name 'objectName'
 *   recursively from the current QML tree item
 *
 * You can apply 0, 1, or more filters to paths.
 * - '[' + number + ']' -> from all the items selected from the path block,
 *   pick the one in position `number`.
 * - '{' + propertyName + '}' - from all the items selected from the path
 *   block, pick the ones that have a property with the name `propertyName`.
 * - '{' + propertyName + '=' + propertyValue + '}' - from all the items
 *   selected from the path block, pick the ones with the property with the name
 *   `propertyName` and value set to `propertyValue`.
 *
 * Paths blocks can be concatenated: `/abc//foo[1]{p=42}/bar`
 */
class QmlPath final {
  struct Filter {
    enum {
      Index,
      Property,
    } m_type;

    int m_index = 0;

    QByteArray m_property;
    bool m_hasPropertyValue = false;
    QString m_propertyValue;
  };

  struct Data {
    bool m_nested = false;
    QString m_key;

    QList<Filter> m_filters;
  };

 public:
  explicit QmlPath(const QString& path);
  ~QmlPath() = default;

  bool isValid() const { return !m_blocks.isEmpty(); }

  QQuickItem* evaluate(QQmlApplicationEngine* engine) const;

 private:
  static bool parsePath(const QChar*& input, qsizetype& size,
                        QList<Data>& blocks);

  QQuickItem* evaluateItems(QQuickItem* currentItem,
                            const QList<QQuickItem*>& items,
                            QList<Data>::const_iterator i) const;

  static QList<QQuickItem*> collectChildItems(QQuickItem* item);

  static QList<QQuickItem*> findItems(QQuickItem* item, const QString& key);

  static QList<QQuickItem*> filterByIndex(const QList<QQuickItem*>& items,
                                          const Filter& filter);
  static void filterByProperty(QList<QQuickItem*>& items, const Filter& filter);

 private:
  QList<Data> m_blocks;
};
