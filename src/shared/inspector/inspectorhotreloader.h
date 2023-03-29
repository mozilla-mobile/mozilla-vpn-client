//
// Created by Basti on 02/03/2023.
//
#ifndef MOZILLA_VPN_INSPECTORHOTRELOADER_H
#define MOZILLA_VPN_INSPECTORHOTRELOADER_H

#include <QApplication>
#include <QObject>
#include <QQmlAbstractUrlInterceptor>
#include <QQmlEngine>

#include "frontend/navigator.h"
#include "frontend/navigatorreloader.h"
#include "logger.h"

class InspectorHotreloader : public QQmlAbstractUrlInterceptor {
 public:
  InspectorHotreloader(QQmlEngine* target) : m_target(target) {
    m_target->addUrlInterceptor(this);
    new NavigatorReloader(qApp);
  }

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

 private:
  QQmlEngine* m_target;
  QMap<QString, QUrl> m_announced_files;
};

#endif  // MOZILLA_VPN_INSPECTORHOTRELOADER_H
