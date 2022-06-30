/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TUTORIAL_H
#define TUTORIAL_H

#include "externalophandler.h"

#include <QList>
#include <QAbstractListModel>

class Addon;
class AddonTutorial;
class QJsonObject;

class Tutorial final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(Tutorial)
  Q_PROPERTY(bool tooltipShown MEMBER m_tooltipShown NOTIFY tooltipShownChanged)
  Q_PROPERTY(bool playing READ isPlaying NOTIFY playingChanged)

 public:
  static Tutorial* instance();

  ~Tutorial();

  Q_INVOKABLE void play(Addon* tutorial);
  Q_INVOKABLE void stop();
  Q_INVOKABLE void allowItem(const QString& objectName);

  bool isPlaying() const { return !!m_currentTutorial; }

  void requireTooltipNeeded(AddonTutorial* tutorial, const QString& tooltipText,
                            QObject* targetElement);
  void requireTutorialCompleted(AddonTutorial* tutorial,
                                const QString& completionMessageText);
  void requireTooltipShown(AddonTutorial* tutorial, bool shown);

 signals:
  void playingChanged();
  void tooltipNeeded(const QString& tooltipText, QObject* targetElement);
  void tooltipShownChanged();
  void tutorialCompleted(const QString& completionMessageText);

 private:
  explicit Tutorial(QObject* parent);

  void externalRequestReceived(ExternalOpHandler::Op op);

  QStringList m_allowedItems;
  bool m_tooltipShown = false;

  AddonTutorial* m_currentTutorial = nullptr;
};

#endif  // TUTORIAL_H
