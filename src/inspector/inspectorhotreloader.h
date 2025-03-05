/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MOZILLA_VPN_INSPECTORHOTRELOADER_H
#define MOZILLA_VPN_INSPECTORHOTRELOADER_H

#include <QApplication>
#include <QQmlAbstractUrlInterceptor>
#include <QQmlEngine>

class InspectorHotreloader : public QQmlAbstractUrlInterceptor {
 public:
  InspectorHotreloader(QQmlEngine* target);

  // Callback for QT.
  QUrl intercept(const QUrl& url,
                 QQmlAbstractUrlInterceptor::DataType type) override;

  /**
   * @brief Announces a qml replacement path is available
   * Will redirect all requests that match path.filename
   * to that URL
   *
   * supported schemes: qrc://, file://
   *
   * @param path - The Replacement Path
   */
  void annonceReplacedFile(const QUrl& path);

  // Removes all injected paths
  // Reload's the current screen with the internal
  // Components
  void resetAllFiles();

  /**
   * @brief Closes and re-opens the QML window
   * forcing a refresh of all components.
   *
   */
  void reloadWindow();

 private:
  QQmlEngine* m_target = nullptr;
  QMap<QString, QUrl> m_announced_files;
};

#endif  // MOZILLA_VPN_INSPECTORHOTRELOADER_H
