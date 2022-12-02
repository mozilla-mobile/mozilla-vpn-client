/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MODULE_H
#define MODULE_H

#include <QObject>

class Module : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(Module)

  Q_PROPERTY(QString name READ name CONSTANT)

 public:
  static void initialize();

  Module(QObject* parent, const QString& name);
  virtual ~Module();

  const QString& name() const { return m_name; }

 private:
  QString m_name;
};

#endif  // MODULE_H
