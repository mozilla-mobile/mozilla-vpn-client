/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef QML_WRAPPER_H
#define QML_WRAPPER_H

#include <QObject>
#include <QPointer>
#include <QProperty>
#include <QQmlApplicationEngine>
#include <QQuickItem>

namespace InspectorTools {

/**
 * QQItemWrapper -
 * "wrappes" a QQuickItem, forwarding all properties
 * omitting field's that are not suited for QWebchannel
 *
 * i.e sending children() + parent() would cause cyclic evaluation
 *
 */
class QQItemWrapper : public QObject {
  Q_OBJECT

 public:
  // Wrap a QQuickItem, may create a QQItemWrapper or
  // return an existing wrapper.
  // Important: This object is destroyed when the QQUickItem is
  // destroyed (which the QMLEngine controlls), always check the pointers
  // validity!
  static QPointer<QQItemWrapper> from(QQuickItem* parent);

  // List of Child Elements, all Wrapped
  Q_PROPERTY(QVariantList children READ wrapChildren CONSTANT)
  // Classname of the QML Element
  Q_PROPERTY(QString className READ getClassName CONSTANT)
 private:
  QVariantList wrapChildren() const;

  QString getClassName() {
    if (m_item == nullptr) {
      return "Unknown";
    }
    auto metaObject = m_item->metaObject();
    return metaObject->className();
  }
  QQItemWrapper(QQuickItem* parent);
  QPointer<QQuickItem> m_item;

  void connectSignals();

#define FORWARDED_PROPERTY(TYPE, NAME)                                        \
  Q_PROPERTY(TYPE NAME READ get_##NAME WRITE set_##NAME NOTIFY NAME##Changed) \
  TYPE get_##NAME() const { return m_item->property(#NAME).value<TYPE>(); }   \
  void set_##NAME(TYPE value) {                                               \
    m_item->setProperty(#NAME, value);                                        \
    emit NAME##Changed();                                                     \
  }                                                                           \
  Q_SIGNAL void NAME##Changed();

  FORWARDED_PROPERTY(bool, activeFocus);
  FORWARDED_PROPERTY(bool, activeFocusOnTab);
  FORWARDED_PROPERTY(bool, antialiasing);
  FORWARDED_PROPERTY(qreal, baselineOffset);
  FORWARDED_PROPERTY(QRectF, childrenRect);
  FORWARDED_PROPERTY(bool, clip);
  FORWARDED_PROPERTY(bool, enabled);
  FORWARDED_PROPERTY(bool, focus);
  FORWARDED_PROPERTY(qreal, height);
  FORWARDED_PROPERTY(qreal, implicitHeight);
  FORWARDED_PROPERTY(qreal, implicitWidth);
  FORWARDED_PROPERTY(qreal, opacity);
  FORWARDED_PROPERTY(qreal, rotation);
  FORWARDED_PROPERTY(qreal, scale);
  FORWARDED_PROPERTY(bool, smooth);
  FORWARDED_PROPERTY(QString, state);
  FORWARDED_PROPERTY(QQuickItem::TransformOrigin, transformOrigin);
  FORWARDED_PROPERTY(bool, visible);
  FORWARDED_PROPERTY(qreal, width);
  FORWARDED_PROPERTY(qreal, x);
  FORWARDED_PROPERTY(qreal, y);
  FORWARDED_PROPERTY(qreal, z);
};

}  // namespace InspectorTools

#endif