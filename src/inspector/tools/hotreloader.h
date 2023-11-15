/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MOZILLA_VPN_INSPECTORHOTRELOADER_H
#define MOZILLA_VPN_INSPECTORHOTRELOADER_H

#include <QApplication>
#include <QQmlAbstractUrlInterceptor>
#include <QQmlEngine>

namespace InspectorTools {

/**
* Hotreloader intercepts all load's
* for a QmlEngine inject's overwrites added 
* at runtine. 
* Can Signal to loaders that a reload is required. 
*/

class Hotreloader : public QQmlAbstractUrlInterceptor {
 public:
  Hotreloader(QQmlEngine* target);

  // Callback for QT.
  QUrl intercept(const QUrl& url,
                 QQmlAbstractUrlInterceptor::DataType type) override;

  /**
   * @brief Announces a qml replacement path is available
   * Will redirect all requests that match path.filename
   * to that URL
   *
   * supported schemes: qrc://, file://, http://
   *
   * @param path - The Replacement Path
   */
  void annonceReplacedFile(const QUrl& path);

  // Removes all injected paths
  // Reload's the current screen with the internal
  // Components
  void resetAllFiles();

  /**
   * @brief Announces a qml replacement path is available
   * Will download a file, store it in a temp directory and
   * redirect all matching files to that.
   *
   * @param path - The Replacement Path
   */
  void fetchAndAnnounce(const QUrl& path);

  /**
   * @brief Closes and re-opens the QML window
   * forcing a refresh of all components.
   *
   */
  void reloadWindow();

 private:
  QQmlEngine* m_target = nullptr;
  QMap<QString, QUrl> m_announced_files;
  QString m_qml_folder;
};

#endif  // MOZILLA_VPN_INSPECTORHOTRELOADER_H
}