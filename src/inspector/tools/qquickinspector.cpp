/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "qquickinspector.h"

#include <QList>
#include <QQuickWindow>

#include "qqitemwrapper.h"

using namespace InspectorTools;


QList<QQItemWrapper*>  QQuickInspector::queryAllElements() { 
    QList<QQItemWrapper*> out; 
    
    QQmlApplicationEngine* engine = qobject_cast<QQmlApplicationEngine*>(m_engine);
    if (!engine) {
      return out;
    }
    for (auto& root : engine->rootObjects()) {
      QQuickWindow* window = qobject_cast<QQuickWindow*>(root);
      if (window == nullptr) {
        continue;
      }
      QQuickItem* content = window->contentItem();
      out.append(QQItemWrapper::from(content));
    }
    return out; 
}
