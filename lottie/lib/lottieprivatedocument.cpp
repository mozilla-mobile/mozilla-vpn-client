/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "lottieprivatedocument.h"

#include <QJSEngine>

#include "lottieprivate.h"

LottiePrivateDocument::LottiePrivateDocument(LottiePrivate* parent)
    : QObject(parent), m_private(parent) {
  Q_ASSERT(parent);
}

QJSValue LottiePrivateDocument::createElement(const QString& type) {
  if (!type.compare("canvas", Qt::CaseInsensitive)) {
    return m_private->engine()->toScriptValue(m_private->canvas());
  }

  qDebug() << "Unable to create element" << type;

  return m_private->engine()->newErrorObject(QJSValue::TypeError,
                                             "Unsupported type");
}

QJSValue LottiePrivateDocument::getElementsByTagName(const QString& tagName) {
  QJSValue array = m_private->engine()->newArray();

  if (!tagName.compare("canvas", Qt::CaseInsensitive) && m_private->canvas()) {
    array.setProperty(0,
                      m_private->engine()->toScriptValue(m_private->canvas()));
  }

  return array;
}

QJSValue LottiePrivateDocument::getElementsByClassName(
    const QString& className) {
  Q_UNUSED(className);
  return m_private->engine()->newArray();
}
