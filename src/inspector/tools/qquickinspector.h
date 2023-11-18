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


  Q_INVOKABLE QQItemWrapper* findByObjectName(const QString& name) {
    QQuickItem* item =
        qobject_cast<QQuickItem*>(findByObjectName(name, m_engine));
    return QQItemWrapper::from(item);
  }
  Q_INVOKABLE QQItemWrapper* queryObject(const QString& path) {
    QQuickItem* item = qobject_cast<QQuickItem*>(queryObject(path, m_engine));
    return QQItemWrapper::from(item);
  }

  /**
  * Queries one Element's in the Provided {engine}
  * by the ObjectName. 
  * 
  * The query may provide nested objectnames "<parentObjectName>/<childObjectName>"
  * for more Complex Queries. 
  * 
  * Please use queryObject to construct complex queries. 
  **/
  static QObject* findByObjectName(const QString& name,
                             QQmlApplicationEngine* engine);

  /**
  * Queries one Element matching the QMLPath {path}
  * See QMLPath.h for more info 
  */
  static QObject* queryObject(const QString& path,
                              QQmlApplicationEngine* engine);

 private:
  QPointer<QQmlApplicationEngine> m_engine;
};

}  // namespace InspectorTools

#endif