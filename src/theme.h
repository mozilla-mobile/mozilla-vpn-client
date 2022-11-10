/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef THEME_H
#define THEME_H

#include <QAbstractListModel>
#include <QHash>
#include <QJSValue>

class QJSEngine;

class Theme final : public QAbstractListModel {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(Theme)

  Q_PROPERTY(QJSValue theme READ readTheme NOTIFY changed)
  Q_PROPERTY(QJSValue colors READ readColors NOTIFY changed)
  Q_PROPERTY(QString currentTheme READ currentTheme WRITE setCurrentTheme NOTIFY
                 changed)

 public:
  Theme();
  ~Theme();

  enum ModelRoles {
    NameRole = Qt::UserRole + 1,
  };

  const QJSValue& readTheme() const;
  const QJSValue& readColors() const;

  const QString& currentTheme() const { return m_currentTheme; }
  void setCurrentTheme(const QString& themeName);

  void initialize(QJSEngine* engine);

  // QAbstractListModel methods

  QHash<int, QByteArray> roleNames() const override;

  int rowCount(const QModelIndex&) const override;

  QVariant data(const QModelIndex& index, int role) const override;

  enum StatusBarTextColor {
    StatusBarTextColorLight,
    StatusBarTextColorDark,
  };
  Q_ENUM(StatusBarTextColor)
  Q_INVOKABLE void setStatusBarTextColor(Theme::StatusBarTextColor color);

 private:
  void parseTheme(QJSEngine* engine, const QString& themeName);
  bool loadTheme(const QString& themeName);

 signals:
  void changed();

 private:
  struct ThemeData {
    QJSValue theme;
    QJSValue colors;
  };

  QHash<QString, ThemeData*> m_themes;
  QString m_currentTheme;
};

#endif  // THEME_H
