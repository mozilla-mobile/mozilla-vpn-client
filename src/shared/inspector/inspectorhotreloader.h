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

  QUrl intercept(const QUrl& url,
                 QQmlAbstractUrlInterceptor::DataType type) override;
  void annonceReplacedFile(const QUrl& path);

 private:
  QQmlEngine* m_target;
  QMap<QString, QUrl> m_announced_files;
};

#endif  // MOZILLA_VPN_INSPECTORHOTRELOADER_H
