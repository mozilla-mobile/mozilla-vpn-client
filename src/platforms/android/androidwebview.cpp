/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "androidwebview.h"
#include "errorhandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkmanager.h"

#include <QAndroidJniEnvironment>
#include <QDebug>
#include <QQuickRenderControl>
#include <QQuickWindow>
#include <QThread>
#include <QWindow>
#include <QtAndroid>

namespace {
Logger logger(LOG_ANDROID, "AndroidWebView");
bool s_methodsInitialized = false;
AndroidWebView* s_instance = nullptr;
constexpr auto WEBVIEW_CLASS = "org/mozilla/firefox/vpn/qt/VPNWebView";
}  // namespace

// static
void AndroidWebView::dispatchToMainThread(std::function<void()> callback) {
  QTimer* timer = new QTimer();
  timer->moveToThread(qApp->thread());
  timer->setSingleShot(true);
  QObject::connect(timer, &QTimer::timeout, [=]() {
    callback();
    timer->deleteLater();
  });
  QMetaObject::invokeMethod(timer, "start", Qt::QueuedConnection);
}

// static
void AndroidWebView::onPageStarted(JNIEnv* env, jobject thiz, jstring url,
                                   jobject icon) {
  Q_UNUSED(env);
  Q_UNUSED(thiz);
  Q_UNUSED(icon);

  QString pageUrl = env->GetStringUTFChars(url, 0);
  logger.debug() << "Page started:" << pageUrl;

  dispatchToMainThread([pageUrl] {
    Q_ASSERT(s_instance);
    emit s_instance->pageStarted(pageUrl);
  });
}

// static
void AndroidWebView::onError(JNIEnv* env, jobject thiz, jint errorCode,
                             jstring description, jstring url) {
  Q_UNUSED(env);
  Q_UNUSED(thiz);
  Q_UNUSED(errorCode);
  Q_UNUSED(description);
  Q_UNUSED(url);

  QString errorDescription = env->GetStringUTFChars(description, 0);
  logger.error() << "Network failure:" << errorDescription;

  dispatchToMainThread([errorDescription] {
    Q_ASSERT(s_instance);
    s_instance->propagateError(ErrorHandler::NoConnectionError);
  });
}

AndroidWebView::AndroidWebView(QQuickItem* parent) : QQuickItem(parent) {
  MVPN_COUNT_CTOR(AndroidWebView);

  logger.debug() << "AndroidWebView created";

  // We do not support multiple android webviews. No needs for now.
  Q_ASSERT(!s_instance);
  s_instance = this;

  if (!s_methodsInitialized) {
    s_methodsInitialized = true;

    QAndroidJniEnvironment env;
    jclass javaClass = env.findClass(WEBVIEW_CLASS);
    if (!javaClass) {
      propagateError(ErrorHandler::RemoteServiceError);
      return;
    }

    JNINativeMethod methods[]{
        {"nativeOnPageStarted",
         "(Ljava/lang/String;Landroid/graphics/Bitmap;)V",
         reinterpret_cast<void*>(onPageStarted)},
        {"nativeOnError", "(ILjava/lang/String;Ljava/lang/String;)V",
         reinterpret_cast<void*>(onError)},
    };

    env->RegisterNatives(javaClass, methods,
                         sizeof(methods) / sizeof(methods[0]));
  }

  QString userAgentStr = NetworkManager::userAgent();
  QAndroidJniObject userAgent = QAndroidJniObject::fromString(userAgentStr);
  Q_ASSERT(userAgent.isValid());

  QAndroidJniObject activity = QtAndroid::androidActivity();
  Q_ASSERT(activity.isValid());

  m_object = QAndroidJniObject(
      WEBVIEW_CLASS, "(Landroid/app/Activity;Ljava/lang/String;)V",
      activity.object<jobject>(), userAgent.object<jstring>());

  if (!m_object.isValid()) {
    propagateError(ErrorHandler::UnrecoverableError);
    return;
  }

  m_webView =
      m_object.callObjectMethod("getWebView", "()Landroid/webkit/WebView;");
  if (!m_webView.isValid()) {
    propagateError(ErrorHandler::UnrecoverableError);
    return;
  }

  m_window = QWindow::fromWinId(reinterpret_cast<WId>(m_webView.object()));

  connect(this, &QQuickItem::windowChanged, this,
          &AndroidWebView::onWindowChanged);
  connect(this, &QQuickItem::visibleChanged, this,
          &AndroidWebView::onVisibleChanged);
}

AndroidWebView::~AndroidWebView() {
  MVPN_COUNT_DTOR(AndroidWebView);

  logger.debug() << "AndroidWebView destroyed";

  Q_ASSERT(s_instance == this);
  s_instance = nullptr;

  if (m_window) {
    m_window->setVisible(false);
    m_window->setParent(0);
    delete m_window;
  }

  if (m_object.isValid()) {
    m_object.callMethod<void>("destroy");
  }
}

QUrl AndroidWebView::url() const {
  if (!m_object.isValid()) {
    logger.warning() << "Invalid object. Returning an empty URL";
    return QUrl();
  }

  return QUrl::fromUserInput(
      m_object.callObjectMethod<jstring>("getUrl").toString());
}

void AndroidWebView::setUrl(const QUrl& url) {
  logger.debug() << "Set URL:" << url.toString();

  if (!m_object.isValid()) {
    logger.error() << "Invalid object. Failed the loading.";
    return;
  }

  QAndroidJniObject urlString = QAndroidJniObject::fromString(url.toString());
  m_object.callMethod<void>("setUrl", "(Ljava/lang/String;)V",
                            urlString.object<jstring>());
  emit urlChanged();
}

void AndroidWebView::componentComplete() {
  if (m_window) {
    m_window->setVisibility(QWindow::Windowed);
  }
}

void AndroidWebView::updatePolish() {
  QSize itemSize = QSize(width(), height());
  if (!itemSize.isValid()) {
    return;
  }

  QQuickWindow* w = window();
  if (!w) {
    return;
  }

  QRect itemGeometry = mapRectToScene(QRect(QPoint(0, 0), itemSize)).toRect();

  const QPoint& tl = w->mapToGlobal(itemGeometry.topLeft());
  QWindow* rw = QQuickRenderControl::renderWindowFor(w);

  m_window->setGeometry(rw ? QRect(rw->mapFromGlobal(tl), itemSize)
                           : itemGeometry);
  m_window->setVisible(isVisible());
}

void AndroidWebView::geometryChanged(const QRectF& newGeometry,
                                     const QRectF& oldGeometry) {
  QQuickItem::geometryChanged(newGeometry, oldGeometry);
  if (newGeometry.isValid()) {
    polish();
  }
}

void AndroidWebView::propagateError(ErrorHandler::ErrorType error) {
  MozillaVPN::instance()->errorHandle(error);
  emit failure();
}

void AndroidWebView::onWindowChanged(QQuickWindow* window) {
  logger.debug() << "window changed";

  QQuickWindow* oldParent = qobject_cast<QQuickWindow*>(m_window->parent());
  if (oldParent) {
    oldParent->disconnect(this);
  }

  if (!window) {
    m_window->setParent(nullptr);
    return;
  }

  // Check if there's an actual native window available.
  QWindow* rw = QQuickRenderControl::renderWindowFor(window);
  if (!rw) {
    rw = window;
  }

  connect(rw, &QWindow::widthChanged, this, &AndroidWebView::polish);
  connect(rw, &QWindow::heightChanged, this, &AndroidWebView::polish);
  connect(rw, &QWindow::xChanged, this, &AndroidWebView::polish);
  connect(rw, &QWindow::yChanged, this, &AndroidWebView::polish);
  connect(rw, &QWindow::visibleChanged, this,
          [this](bool visible) { m_window->setVisible(visible); });
  connect(window, &QQuickWindow::sceneGraphInitialized, this,
          &AndroidWebView::polish);
  connect(window, &QQuickWindow::sceneGraphInvalidated, this,
          &AndroidWebView::invalidateSceneGraph);
  m_window->setParent(rw);
}

void AndroidWebView::onVisibleChanged() {
  logger.debug() << "visible changed";
  m_window->setVisible(isVisible());
}

void AndroidWebView::invalidateSceneGraph() {
  if (m_window) {
    m_window->setVisible(false);
  }
}
