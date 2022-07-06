/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TUTORIALMODEL_H
#define TUTORIALMODEL_H

#include "tutorial.h"
#include "externalophandler.h"

#include <QList>
#include <QAbstractListModel>

class QJsonObject;
class Tutorial;

class TutorialModel final : public QAbstractListModel {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(TutorialModel)
  Q_PROPERTY(bool tooltipShown MEMBER m_tooltipShown NOTIFY tooltipShownChanged)
  Q_PROPERTY(bool playing READ isPlaying NOTIFY playingChanged)
  Q_PROPERTY(Tutorial* highlightedTutorial READ highlightedTutorial NOTIFY
                 highlightedTutorialChanged)
  Q_PROPERTY(
      Tutorial* currentTutorial MEMBER m_currentTutorial NOTIFY playingChanged)

 public:
  enum ModelRoles {
    TutorialRole = Qt::UserRole + 1,
  };

  static TutorialModel* instance();

  ~TutorialModel();

  Q_INVOKABLE void play(Tutorial* tutorial);
  Q_INVOKABLE void stop();
  Q_INVOKABLE void allowItem(const QString& objectName);

  bool isPlaying() const { return !!m_currentTutorial; }

  void requireTooltipNeeded(Tutorial* tutorial, const QString& tooltipText,
                            QObject* targetElement);
  void requireTutorialCompleted(Tutorial* tutorial,
                                const QString& completionMessageText);
  void requireTooltipShown(Tutorial* tutorial, bool shown);

  Tutorial* highlightedTutorial() const;

  bool createFromJson(const QString& addonId, const QJsonObject& obj);
  void remove(const QString& addonId);

  // QAbstractListModel methods

  QHash<int, QByteArray> roleNames() const override;

  int rowCount(const QModelIndex&) const override;

  QVariant data(const QModelIndex& index, int role) const override;

 signals:
  void playingChanged();
  void tooltipNeeded(const QString& tooltipText, QObject* targetElement);
  void tooltipShownChanged();
  void tutorialCompleted(const QString& completionMessageText);

 signals:
  void highlightedTutorialChanged();

 private:
  explicit TutorialModel(QObject* parent);

  void externalRequestReceived(ExternalOpHandler::Op op);

  struct TutorialData {
    QString m_addonId;
    Tutorial* m_tutorial;
  };

  QList<TutorialData> m_tutorials;
  Tutorial* m_currentTutorial = nullptr;

  QStringList m_allowedItems;
  bool m_tooltipShown = false;
};

#endif  // TUTORIALMODEL_H
