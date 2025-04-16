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
class XdgAppearance;

class Theme final : public QAbstractListModel {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(Theme)

  // sizingChanged isn't currently used, by QML constantly spams warnings if
  // there is no NOTIFY for `theme`
  Q_PROPERTY(QJSValue theme READ readTheme NOTIFY sizingChanged)
  Q_PROPERTY(QJSValue colors READ readColors NOTIFY changed)
  Q_PROPERTY(QString currentTheme READ currentTheme WRITE setCurrentTheme NOTIFY
                 changed)

 public:
  explicit Theme(QObject* parent);
  ~Theme();

  static Theme* instance();

  enum ModelRoles {
    NameRole = Qt::UserRole + 1,
  };

  const QJSValue& readTheme() const;
  const QJSValue readColors() const;

  const QString& currentTheme() const { return m_currentTheme; }

  // Todo: Add a thing for themes to define, if they are using dark or light
  // resources. `useDarkAssets` is available, add this to ThemeData and connect.
  Q_INVOKABLE bool isThemeDark() { return m_currentTheme != "main"; }

  void setCurrentTheme(const QString& themeName);

  Q_INVOKABLE void setUsingSystemTheme(const bool usingSystemTheme);

  void initialize(QJSEngine* engine);

  // QAbstractListModel methods

  QHash<int, QByteArray> roleNames() const override;

  int rowCount(const QModelIndex&) const override;

  QVariant data(const QModelIndex& index, int role) const override;

  QString currentSystemTheme();

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
  void parseTheme(QJSEngine* engine, const QString& themeFilename);
  bool loadTheme(const QString& themeName);
  void parseSizing(QJSEngine* engine);
  void setToSystemTheme();

 signals:
  void changed();
  void sizingChanged();

 private:
  QHash<QString, QJSValue> m_themes;
  QString m_currentTheme;
  QJSValue m_sizing;

#if defined(MZ_LINUX)
  XdgAppearance* m_xdg = nullptr;
#endif
};

#endif  // THEME_H
