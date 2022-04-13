/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ANDROIDGLEAN_H
#define ANDROIDGLEAN_H

#include <jni.h>
#include <QObject>
#include <QString>


#include "androidjnicompat.h"

class AndroidGlean final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AndroidGlean)
public: 
  // For Glean
  void initializeGlean();
  void sendGleanPings();
  void recordGleanEvent(const QString& gleanSampleName);
  void gleanEnabledChanged(bool enabled);
  void recordGleanEventWithExtraKeys(const QString& gleanSampleName,
                                     const QVariantMap& extraKeys);
  void setGleanSourceTags(const QStringList& tags);

 static void initialize();
 private:
  AndroidGlean(QObject* parent);
  ~AndroidGlean();
};

#endif  // ANDROIDGLEAN_H
