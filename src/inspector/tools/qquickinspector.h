/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef QT_GRAPH_H
#define QT_GRAPH_H

#include <QObject>
#include <QPointer>
#include <QQmlApplicationEngine>

#include "qqitemwrapper.h"

namespace InspectorTools {

class QQItemWrapper;

/**
  Exposes API's to query the rendered QtQML Elements
*/
class QQuickInspector : public QObject {
  Q_OBJECT
 public:
     // Construct a new Inspector, all Elements are scoped to the QQmlAppEngine. 
  QQuickInspector(QObject* parent, QQmlApplicationEngine* engine)
      : QObject(parent), m_engine(engine){};
    // Returns all Elements, wrapped in QQItemwrappers 
  Q_INVOKABLE QList<QQItemWrapper*> queryAllElements();

 private:
  QPointer<QQmlApplicationEngine> m_engine;
};

}  // namespace InspectorTools
   // namespace InspectorTools

#endif