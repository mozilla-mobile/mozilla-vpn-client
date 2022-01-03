/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LOTTIEPRIVATEDOCUMENT_H
#define LOTTIEPRIVATEDOCUMENT_H

#include <QJSValue>
#include <QObject>

class LottiePrivate;
class QQuickItem;

// This is a simple "DOM document" implementation.
class LottiePrivateDocument final : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString readyState READ readyState CONSTANT)

 public:
  explicit LottiePrivateDocument(LottiePrivate* parent);

  QString readyState() const { return "complete"; }

  Q_INVOKABLE QJSValue createElement(const QString& type);
  Q_INVOKABLE QJSValue getElementsByTagName(const QString& tagName);
  Q_INVOKABLE QJSValue getElementsByClassName(const QString& className);

 private:
  LottiePrivate* m_private = nullptr;
};

#endif  // LOTTIEPRIVATEDOCUMENT_H
