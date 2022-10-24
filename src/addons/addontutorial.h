/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADDONTUTORIAL_H
#define ADDONTUTORIAL_H

#include "addon.h"
#include "addonproperty.h"

class NavigatorReloader;
class QJsonObject;
class QQuickItem;
class TutorialStep;

class AddonTutorial final : public Addon {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AddonTutorial)

 public:
  ADDON_PROPERTY(title, m_title, getTitle, setTitle, retranslationCompleted)
  ADDON_PROPERTY(subtitle, m_subtitle, getSubtitle, setSubtitle,
                 retranslationCompleted)
  ADDON_PROPERTY(completionMessage, m_completionMessage, getCompletionMessage,
                 setCompletionMessage, retranslationCompleted)

  Q_PROPERTY(QString image MEMBER m_image CONSTANT)
  Q_PROPERTY(bool highlighted READ highlighted CONSTANT)

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
  bool maybeStop();

 private:
  AddonProperty m_title;
  AddonProperty m_subtitle;
  AddonProperty m_completionMessage;
  QString m_image;

  QList<TutorialStep*> m_steps;
  int32_t m_currentStep = -1;
  bool m_highlighted = false;

  QStringList m_allowedItems;

  class TutorialItemPicker;
  TutorialItemPicker* m_itemPicker = nullptr;

  NavigatorReloader* m_navigatorReloader = nullptr;
};

#endif  // ADDONTUTORIAL_H
