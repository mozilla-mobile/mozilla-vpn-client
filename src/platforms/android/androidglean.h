/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ANDROIDGLEAN_H
#define ANDROIDGLEAN_H

#include <jni.h>
#include <QObject>
#include <QString>
#include <QQmlEngine>

class AndroidGlean final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AndroidGlean)
 public:
  // For Glean
  void sendGleanMainPings();
  void recordGleanEvent(const QString& gleanSampleName);
  void gleanUploadEnabledChanged();
  void recordGleanEventWithExtraKeys(const QString& gleanSampleName,
                                     const QVariantMap& extraKeys);
  void setGleanSourceTags(const QStringList& tags);

  void applicationStateChanged(Qt::ApplicationState state);

  void daemonConnected();

  static void initialize(QQmlEngine* engine);

 private:
  explicit AndroidGlean(QObject* parent);
  ~AndroidGlean();
};

#endif  // ANDROIDGLEAN_H
