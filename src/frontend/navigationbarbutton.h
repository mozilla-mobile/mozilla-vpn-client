/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef NAVIGATIONBARBUTTON_H
#define NAVIGATIONBARBUTTON_H

#include <QObject>

class NavigationBarButton final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(NavigationBarButton)

  Q_PROPERTY(QString objectName MEMBER m_objectName CONSTANT);
  Q_PROPERTY(QString navAccessibleName MEMBER m_navAccessibleName CONSTANT);
  Q_PROPERTY(QString source READ source NOTIFY sourceChanged);
  Q_PROPERTY(int screen READ screen CONSTANT);
  Q_PROPERTY(bool checked READ checked WRITE setChecked NOTIFY sourceChanged);

 public:
  NavigationBarButton(
      QObject* parent, const QString& objectName,
      const QString& navAccessibleName, int screen,
      const QString& sourceUnchecked, const QString& sourceChecked,
      const QString& sourceUncheckedDark, const QString& sourceCheckedDark,
      const QString& sourceUncheckedNotification = QString(),
      const QString& sourceCheckedNotification = QString(),
      const QString& sourceUncheckedNotificationDark = QString(),
      const QString& sourceCheckedNotificationDark = QString());
  ~NavigationBarButton();

  const QString& source() const;

  bool checked() const { return m_checked; }
  void setChecked(bool checked);

  void setHasNotification(bool hasNotification);
  void updateButton();

  int screen() const { return m_screen; }

 signals:
  void sourceChanged();

 private:
  const QString m_objectName;
  const QString m_navAccessibleName;
  const QString m_sourceUnchecked;
  const QString m_sourceChecked;
  const QString m_sourceUncheckedDark;
  const QString m_sourceCheckedDark;
  const QString m_sourceUncheckedNotification;
  const QString m_sourceCheckedNotification;
  const QString m_sourceUncheckedNotificationDark;
  const QString m_sourceCheckedNotificationDark;
  const int m_screen;

  bool m_checked = false;
  bool m_hasNotification = false;
};

#endif  // NAVIGATIONBARBUTTON_H
