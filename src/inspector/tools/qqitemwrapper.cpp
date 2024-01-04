/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "qqitemwrapper.h"
#include <QTest>



namespace {
QMap<QQuickItem*, QPointer<InspectorTools::QQItemWrapper>> element_cache;
}


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

QQItemWrapper::QQItemWrapper(QQuickItem* parent) : QObject(parent), m_item(parent) {
  connect(parent, &QObject::destroyed, this, &QObject::deleteLater);
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
#define FORWARDED_SIGNAL(TYPE, NAME) \
  connect(m_item, &QQuickItem::NAME##Changed, this, &QQItemWrapper::NAME##Changed);

  FORWARDED_SIGNAL(bool, activeFocus)
  FORWARDED_SIGNAL(bool, activeFocusOnTab)
  FORWARDED_SIGNAL(bool, antialiasing)
  FORWARDED_SIGNAL(qreal, baselineOffset)
  FORWARDED_SIGNAL(QRectF, childrenRect)
  FORWARDED_SIGNAL(bool, clip)
  FORWARDED_SIGNAL(bool, enabled)
  FORWARDED_SIGNAL(bool, focus)
  FORWARDED_SIGNAL(qreal, height)
  FORWARDED_SIGNAL(qreal, implicitHeight)
  FORWARDED_SIGNAL(qreal, implicitWidth)
  FORWARDED_SIGNAL(qreal, opacity)
  FORWARDED_SIGNAL(qreal, rotation)
  FORWARDED_SIGNAL(qreal, scale)
  FORWARDED_SIGNAL(bool, smooth)
  FORWARDED_SIGNAL(QString, state)
  FORWARDED_SIGNAL(QQuickItem::TransformOrigin, transformOrigin)
  FORWARDED_SIGNAL(bool, visible)
  FORWARDED_SIGNAL(qreal, width)
  FORWARDED_SIGNAL(qreal, x)
  FORWARDED_SIGNAL(qreal, y)
  FORWARDED_SIGNAL(qreal, z)


	  #undef FORWARD_SIGNAL



};


void QQItemWrapper::click(){
  QPointF pointF = m_item->mapToScene(QPoint(0, 0));
  QPoint point = pointF.toPoint();
  QWindow* win = (QWindow*)(m_item->window());
  QTest::mouseClick(win, Qt::LeftButton, Qt::NoModifier, point, -1);

}
}  // namespace InspectorTools
