/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef GLEANDEPRECATED_H
#define GLEANDEPRECATED_H

#include <QObject>
#include <QVariant>

class GleanDeprecated final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(GleanDeprecated)

 public:
  static GleanDeprecated* instance();
  ~GleanDeprecated();

 private:
  explicit GleanDeprecated(QObject* parent);

 signals:
  void recordGleanEvent(const QString& gleanSampleName);
  void recordGleanEventWithExtraKeys(const QString& gleanSampleName,
                                     const QVariantMap& extraKeys);
};

#endif  // GLEANDEPRECATED_H
