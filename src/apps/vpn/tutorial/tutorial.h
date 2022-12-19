/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TUTORIAL_H
#define TUTORIAL_H

#include <QAbstractListModel>
#include <QList>

#include "addons/addontutorial.h"
#include "externalophandler.h"

class Addon;
class QJsonObject;

class Tutorial final : public QObject, public ExternalOpHandler::Blocker {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(Tutorial)
  Q_PROPERTY(bool tooltipShown MEMBER m_tooltipShown NOTIFY tooltipShownChanged)
  Q_PROPERTY(bool playing READ isPlaying NOTIFY playingChanged)
  Q_PROPERTY(AddonTutorial* currentTutorial MEMBER m_currentTutorial NOTIFY
                 playingChanged)

 public:
  static Tutorial* instance();

  ~Tutorial();

  Q_INVOKABLE void play(Addon* tutorial);
  Q_INVOKABLE void showWarning(Addon* tutorial);
  Q_INVOKABLE void stop();
  Q_INVOKABLE void allowItem(const QString& objectName);
  Q_INVOKABLE void interruptAccepted(ExternalOpHandler::Op op);

  bool isPlaying() const { return !!m_currentTutorial; }

  void requireTooltipNeeded(AddonTutorial* tutorial, const QString& stepId,
                            const QString& value, QObject* targetElement);
  void requireTutorialCompleted(AddonTutorial* tutorial);
  void requireTooltipShown(AddonTutorial* tutorial, bool shown);

  // ExternalOpHandler::Blocker
  bool maybeBlockRequest(ExternalOpHandler::Op op) override;

 signals:
  void playingChanged();
  void tooltipNeeded(const QString& value, QObject* targetElement);
  void tooltipShownChanged();
  void tutorialCompleted(Addon* tutorial);
  void interruptRequest(ExternalOpHandler::Op op);
  void showWarningNeeded(Addon* tutorial);

 private:
  explicit Tutorial(QObject* parent);

  QStringList m_allowedItems;
  bool m_tooltipShown = false;

  AddonTutorial* m_currentTutorial = nullptr;
};

#endif  // TUTORIAL_H
