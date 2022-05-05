/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CLOSEEVENTHANDLER_H
#define CLOSEEVENTHANDLER_H

#include <QList>
#include <QObject>

class QQuickItem;

class CloseEventHandler final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(CloseEventHandler)

 public:
  CloseEventHandler();
  ~CloseEventHandler();

  Q_INVOKABLE bool eventHandled();
  Q_INVOKABLE void addStackView(const QVariant& stackView);
  Q_INVOKABLE void addView(const QVariant& view);

  Q_INVOKABLE void removeAllStackViews();

 signals:
  void goBack(QQuickItem* item);

 private slots:
  void removeItem(QObject* item);

 private:
  struct Layer {
    enum Type {
      eStackView,
      eView,
    };

    Layer(QQuickItem* layer, Type type) : m_layer(layer), m_type(type) {}

    QQuickItem* m_layer;
    Type m_type;
  };

  QList<Layer> m_layers;
};

#endif  // CLOSEEVENTHANDLER_H
