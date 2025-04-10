/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef NAVIGATOR_H
#define NAVIGATOR_H

#include <QObject>
#include <QQmlComponent>

class QQuickItem;

class Navigator final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(Navigator)

  Q_PROPERTY(int screen READ currentScreen NOTIFY currentComponentChanged)
  Q_PROPERTY(LoadPolicy loadPolicy MEMBER m_currentLoadPolicy NOTIFY
                 currentComponentChanged)
  Q_PROPERTY(LoadingFlags loadingFlags MEMBER m_currentLoadingFlags NOTIFY
                 currentComponentChanged)
  Q_PROPERTY(QQmlComponent* component MEMBER m_currentComponent NOTIFY
                 currentComponentChanged)

 public:
  enum LoadPolicy {
    LoadPersistently,
    LoadTemporarily,
  };
  Q_ENUM(LoadPolicy);

  enum LoadingFlags {
    NoFlags,
    ForceReload,
    ForceReloadAll,
  };
  Q_ENUM(LoadingFlags);

  enum Screen : int {
    // This enum will be populated with shareable screens when ready.
    ScreenCustom = 1000
  };
  Q_ENUM(Screen);

  static Navigator* instance();

  int currentScreen() const { return m_currentScreen; }

  void initialize();

  ~Navigator();

  Q_INVOKABLE void requestScreenFromBottomBar(
      int screen, Navigator::LoadingFlags loadingFlags = NoFlags);
  Q_INVOKABLE void requestScreen(
      int screen, Navigator::LoadingFlags loadingFlags = NoFlags);
  Q_INVOKABLE void requestDeepLink(const QUrl& url);
  Q_INVOKABLE void requestPreviousScreen();

  Q_INVOKABLE void addStackView(int screen, const QVariant& stackView);
  Q_INVOKABLE void addView(int screen, const QVariant& view);

  Q_INVOKABLE bool eventHandled();

  /**
   * @brief Request's the Load of the current screen with
   * ForceReloadAll policy
   *
   */
  void reloadCurrentScreen();

  static void registerScreen(int screenId, LoadPolicy loadPolicy,
                             const QString& qmlComponentUrl,
                             const QVector<int>& requiresAppState,
                             int8_t (*priorityGetter)(int*),
                             bool (*quitBlocked)());

 signals:
  void goBack(QQuickItem* item);
  void currentComponentChanged();

 private:
  explicit Navigator(QObject* parent);

  void computeComponent();
  void loadScreen(int screen, LoadPolicy loadPolicy, QQmlComponent* component,
                  LoadingFlags loadingFlags);

  void removeItem(QObject* obj);

 private:
  int m_currentScreen = -1;
  LoadPolicy m_currentLoadPolicy = LoadTemporarily;
  LoadingFlags m_currentLoadingFlags = NoFlags;
  QQmlComponent* m_currentComponent = nullptr;

  QList<int> m_screenHistory;

};

#endif  // NAVIGATOR_H
