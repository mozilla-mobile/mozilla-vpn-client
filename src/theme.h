/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef THEME_H
#define THEME_H

#include <QAbstractListModel>
#include <QHash>
#include <QIcon>
#include <QJSValue>

#include "settingsholder.h"

class QJSEngine;

class Theme final : public QAbstractListModel {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(Theme)

  Q_PROPERTY(QJSValue theme READ readTheme NOTIFY changed)
  Q_PROPERTY(QJSValue colors READ readColors NOTIFY changed)
  Q_PROPERTY(QString currentTheme READ currentTheme WRITE setCurrentTheme NOTIFY
                 changed)
  Q_PROPERTY(bool usingSystemTheme READ usingSystemTheme WRITE
                 setUsingSystemTheme NOTIFY changed)

 public:
  explicit Theme(QObject* parent);
  ~Theme();

  static Theme* instance();

  enum ModelRoles {
    NameRole = Qt::UserRole + 1,
  };

  const QJSValue& readTheme() const;
  const QJSValue& readColors() const;

  const QString& currentTheme() const { return m_currentTheme; }
  const bool usingSystemTheme() const {
    return SettingsHolder::instance()->usingSystemTheme();
  }

  // Todo: Add a thing for themes to define, if they are using dark or light
  // resources. `useDarkAssets` is available, add this to ThemeData and connect.
  bool isThemeDark() { return m_currentTheme != "main"; }

  void setCurrentTheme(const QString& themeName);

  void setUsingSystemTheme(const bool usingSystemTheme);

  void initialize(QJSEngine* engine);

  // QAbstractListModel methods

  QHash<int, QByteArray> roleNames() const override;

  int rowCount(const QModelIndex&) const override;

  QVariant data(const QModelIndex& index, int role) const override;

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
  Qt::ColorScheme currentSystemTheme();
#endif

  enum StatusBarTextColor {
    StatusBarTextColorLight,
    StatusBarTextColorDark,
  };
  Q_ENUM(StatusBarTextColor)
  Q_INVOKABLE void setStatusBarTextColor(Theme::StatusBarTextColor color);

  bool usesDarkModeAssets() const;
  // Returns an Icon matching the current colorscheme
  QImage getTitleBarIcon();

 private:
  void parseTheme(QJSEngine* engine, const QString& themeName);
  bool loadTheme(const QString& themeName);
  void setToSystemTheme();

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
