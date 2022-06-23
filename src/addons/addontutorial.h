/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADDONTUTORIAL_H
#define ADDONTUTORIAL_H

#include "addon.h"

class QJsonObject;
class QQuickItem;
class TutorialStep;

class AddonTutorial final : public Addon {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AddonTutorial)

  Q_PROPERTY(QString titleId MEMBER m_titleId CONSTANT)
  Q_PROPERTY(QString subtitleId MEMBER m_subtitleId CONSTANT)
  Q_PROPERTY(QString completionMessageId MEMBER m_completionMessageId CONSTANT)
  Q_PROPERTY(QString image MEMBER m_image CONSTANT)
  Q_PROPERTY(bool highlighted READ highlighted CONSTANT)

 public:
  static Addon* create(QObject* parent, const QString& manifestFileName,
                       const QString& id, const QString& name,
                       const QJsonObject& obj);

  ~AddonTutorial();

  void play(const QStringList& allowedItems);
  void stop();

  const QStringList& allowedItems() const { return m_allowedItems; }

  bool highlighted() const { return m_highlighted; }

  bool itemPicked(const QList<QQuickItem*>& list);

 private:
  AddonTutorial(QObject* parent, const QString& manifestFileName,
                const QString& id, const QString& name);

  void processNextOp();

  // Return true if there are no operations left.
  bool maybeStop(bool completed = false);

 private:
  QString m_titleId;
  QString m_subtitleId;
  QString m_completionMessageId;
  QString m_image;

  QList<TutorialStep*> m_steps;
  int32_t m_currentStep = -1;
  bool m_highlighted = false;

  QStringList m_allowedItems;

  class TutorialItemPicker;
  TutorialItemPicker* m_itemPicker = nullptr;
};

#endif  // ADDONTUTORIAL_H
