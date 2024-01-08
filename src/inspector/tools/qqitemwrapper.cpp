/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "qqitemwrapper.h"

#include <QQmlProperty>
#include <QTest>

namespace {
QMap<QQuickItem*, QPointer<InspectorTools::QQItemWrapper>> element_cache;

/**
 * Properties we do not want to serialize,
 * as we either have them already bound in the QObject
 * or it may cause a cyclical resoloution chain.
 */
QStringList FORBIDDEN_KEYS(
    {"activeFocus", "clip", "data", "focus", "height", "opacity", "rotation",
     "scale", "state", "visible", "width", "x", "y", "z", "parent", "children",
     "visibleChildren",

    });

// Types that QWebchannel cannot wrap as they are cyclical.
QStringList FORBIDDEN_TYPES({"QQmlComponent*", "QQuickItem*",
                             "QQuickItemLayer*", "QObject*",
    "QQuickPalette*",
    "QQuickItem::TransformOrigin", 
    "QQmlListProperty<QQuickTransform>",
    "QQuickPen*",
    "QVariant", 
    "QQmlListProperty<QObject>", 
    "QQmlListProperty<QQuickState>" , 
    "QQuickAnchors*"
    });
}  // namespace

namespace InspectorTools {

// Static
QPointer<QQItemWrapper> QQItemWrapper::from(QQuickItem* parent) {
  if (parent == nullptr) {
    return nullptr;
  }
  if (element_cache.contains(parent)) {
    auto candidate = element_cache.value(parent);
    if (!candidate.isNull()) {
      return candidate;
    }
    element_cache.remove(parent);
  }
  auto wrapper = new QQItemWrapper(parent);
  element_cache.insert(parent, QPointer(wrapper));
  return wrapper;
}

QQItemWrapper::QQItemWrapper(QQuickItem* parent)
    : QObject(parent), m_item(parent) {
  connect(parent, &QObject::destroyed, this, &QObject::deleteLater);
  this->setObjectName(m_item->objectName());
  connectSignals();
};

QVariantList QQItemWrapper::wrapChildren() const {
  QVariantList out;
  auto children = m_item->childItems();
  for (auto c : children) {
    auto wrapper = QQItemWrapper::from(c);
    auto variant = QVariant::fromValue(wrapper);
    out.append(variant);
  }
  return out;
}

void QQItemWrapper::connectSignals() {
  // Connect all signals we're forwarding
#define FORWARDED_SIGNAL(TYPE, NAME)                \
  connect(m_item, &QQuickItem::NAME##Changed, this, \
          &QQItemWrapper::NAME##Changed);

  FORWARDED_SIGNAL(bool, activeFocus)
  FORWARDED_SIGNAL(bool, enabled)
  FORWARDED_SIGNAL(bool, focus)
  FORWARDED_SIGNAL(qreal, height)
  FORWARDED_SIGNAL(qreal, opacity)
  FORWARDED_SIGNAL(qreal, rotation)
  FORWARDED_SIGNAL(QString, state)
  FORWARDED_SIGNAL(bool, visible)
  FORWARDED_SIGNAL(qreal, width)
  FORWARDED_SIGNAL(qreal, x)
  FORWARDED_SIGNAL(qreal, y)
  FORWARDED_SIGNAL(qreal, z)
#undef FORWARD_SIGNAL
};

void QQItemWrapper::click() {
  QPointF pointF = m_item->mapToScene(QPoint(0, 0));
  QPoint point = pointF.toPoint();
  QWindow* win = (QWindow*)(m_item->window());
  QTest::mouseClick(win, Qt::LeftButton, Qt::NoModifier, point, -1);
}

QVariantMap QQItemWrapper::dynamicProperties() const {
  QVariantMap out;
  auto names = m_item->dynamicPropertyNames();
  for (const auto name : names) {
    QQmlProperty property(m_item, name);
    qDebug() << name;
    out.insert(name, property.read());
  }
  return out;
}

QVariantMap QQItemWrapper::staticProperties() const {
  QVariantMap out;

  auto mo = m_item->metaObject();
  auto count = mo->propertyCount();

  for (int i = 0; i < count; i++) {
    auto prop = mo->property(i);
    if (FORBIDDEN_KEYS.contains(prop.name())) {
      continue;
    }
    if (FORBIDDEN_TYPES.contains(prop.typeName())) {
      continue;
    }

    qDebug() << "INCLUDING" << prop.name() << prop.typeName();
    out.insert(prop.name(), prop.read(m_item));
  }
  return out;
}

}  // namespace InspectorTools
