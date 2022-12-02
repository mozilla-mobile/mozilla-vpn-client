/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MODULE_H
#define MODULE_H

#include <QObject>

class Module : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(Module)

 public:
  static void load(QObject* parent);

  explicit Module(QObject* parent);
  virtual ~Module();

  virtual void initialize() = 0;
};

#endif  // MODULE_H
