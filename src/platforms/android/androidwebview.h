/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ANDROIDWEBVIEW_H
#define ANDROIDWEBVIEW_H

#include "errorhandler.h"

#include "androidjnicompat.h"

#include <QQuickItem>
#include <QQuickWindow>
#include <QWindow>
#include <QUrl>

class AndroidWebView : public QQuickItem {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AndroidWebView)

  Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged)
  QML_ELEMENT

#if QT_VERSION < 0x060000
  typedef QAndroidJniObject QJniObject;
#endif

 public:
  AndroidWebView(QQuickItem* parent = 0);
  virtual ~AndroidWebView();

  QUrl url() const;
  void setUrl(const QUrl& url);
  Q_INVOKABLE void clearStorage();

 protected:
  void componentComplete() override;
  void updatePolish() override;
#if QT_VERSION < 0x060000
  void geometryChanged(const QRectF& newGeometry,
                       const QRectF& oldGeometry) override;
#else
  void geometryChange(const QRectF& newGeometry,
                      const QRectF& oldGeometry) override;
#endif

 signals:
  void urlChanged();
  void pageStarted(const QString& url);
  void failure();

 private slots:
  void onWindowChanged(QQuickWindow* window);
  void onVisibleChanged();
  void invalidateSceneGraph();

 private:
  static void onPageStarted(JNIEnv* env, jobject thiz, jstring url,
                            jobject icon);
  static void onError(JNIEnv* env, jobject thiz, jint errorCode,
                      jstring description, jstring url);
  void propagateError(ErrorHandler::ErrorType error);

 private:
  QWindow* m_window = nullptr;

  QJniObject m_object;
  QJniObject m_webView;
};

#endif  // ANDROIDWEBVIEW_H
