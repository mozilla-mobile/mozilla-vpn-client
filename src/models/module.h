/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MODULE_H
#define MODULE_H

#include <functional>
#include <QObject>
#include <QVector>

class Module : public QObject {
  Q_OBJECT

  Q_PROPERTY(QString name READ name CONSTANT)
  Q_PROPERTY(QString qmlView READ qmlView CONSTANT)
  Q_PROPERTY(QString qmlWidget READ qmlWidget CONSTANT)

 public:
  Module(const QString& name, const QString& featureId, const QString& qmlView,
         const QString& qmlWidget);
  virtual ~Module() = default;

  static QList<Module*> createModules();

  const QString& name() const { return m_name; }
  const QString& featureId() const { return m_featureId; }
  const QString& qmlView() const { return m_qmlView; }
  const QString& qmlWidget() const { return m_qmlWidget; }

 private:
  QString m_name;
  QString m_featureId;
  QString m_qmlView;
  QString m_qmlWidget;

 protected:
  static QVector<std::function<Module*()>> s_moduleCreators;

 public:
  template <class T>
  struct RegistrationProxy {
    RegistrationProxy() { s_moduleCreators.append(RegistrationProxy::create); }

    static Module* create() { return new T(); }
  };
};

#endif  // MODULE_H
