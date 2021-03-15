/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ANDROIDCAPTIVEPORTALDETECTION_H
#define ANDROIDCAPTIVEPORTALDETECTION_H
#include <QObject>

class AndroidIntentReciver final : public QObject {
  Q_OBJECT

 public:
  AndroidIntentReciver(QObject* parent);

  static void Init();

 public slots:
  void applicationStateChanged(Qt::ApplicationState state);

 private:
  void readCurrentIntent();
};

#endif
