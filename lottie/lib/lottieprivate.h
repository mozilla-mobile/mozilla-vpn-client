/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LOTTIEPRIVATE_H
#define LOTTIEPRIVATE_H

#include "lottiestatus.h"

#include <QJSValue>
#include <QtQuick/QQuickItem>

class QJSEngine;
class LottiePrivateWindow;

class LottiePrivate : public QQuickItem {
  Q_OBJECT
  Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)
  Q_PROPERTY(bool readyToPlay READ readyToPlay WRITE setReadyToPlay NOTIFY
                 readyToPlayChanged)
  Q_PROPERTY(qreal speed READ speed WRITE setSpeed NOTIFY speedChanged)
  Q_PROPERTY(QJSValue loops READ loops WRITE setLoops NOTIFY loopsChanged)
  Q_PROPERTY(bool reverse READ reverse WRITE setReverse NOTIFY reverseChanged)
  Q_PROPERTY(QJSValue status READ status CONSTANT)
  Q_PROPERTY(
      bool autoPlay READ autoPlay WRITE setAutoPlay NOTIFY autoPlayChanged)
  Q_PROPERTY(
      QString fillMode READ fillMode WRITE setFillMode NOTIFY fillModeChanged)
  QML_ELEMENT

 public:
  static void initialize(QJSEngine* engine, const QString& userAgent);
  static const QString& userAgent();
  static QJSEngine* engine();

  LottiePrivate(QQuickItem* parent = 0);

  Q_INVOKABLE void setCanvasAndContainer(QQuickItem* canvas,
                                         QQuickItem* container);
  Q_INVOKABLE void clearAndResize();
  Q_INVOKABLE void destroyAndRecreate();

  Q_INVOKABLE void play();
  Q_INVOKABLE void pause();
  Q_INVOKABLE void stop();

  Q_INVOKABLE void eventPlayingCompleted();
  Q_INVOKABLE void eventLoopCompleted();
  Q_INVOKABLE void eventEnterFrame(const QJSValue& value);

  const QString& source() const { return m_source; }
  void setSource(const QString& source);

  bool readyToPlay() const { return m_readyToPlay; }
  void setReadyToPlay(bool readyToPlay);

  qreal speed() const { return m_speed; }
  void setSpeed(qreal speed);

  QJSValue loops() const { return m_loops; }
  void setLoops(QJSValue loops);

  bool reverse() const { return m_reverse; }
  void setReverse(bool reverse);

  QJSValue status();

  bool autoPlay() const { return m_autoPlay; }
  void setAutoPlay(bool autoPlay);

  const QString& fillMode() const { return m_fillMode; }
  void setFillMode(const QString& fillMode);

  QQuickItem* canvas() const { return m_canvas; }

  QJSValue lottieInstance() const { return m_lottieInstance; }
  void setLottieInstance(QJSValue lottie) { m_lottieInstance = lottie; }

 signals:
  void sourceChanged();
  void readyToPlayChanged();
  void speedChanged();
  void loopsChanged();
  void reverseChanged();
  void autoPlayChanged();
  void fillModeChanged();
  void loopCompleted();

 private:
  QJSValue createWindowObject();
  QJSValue createNavigatorObject();
  QJSValue createDocumentObject();

  void applySpeed();
  void applyDirection();
  void destroyAnimation();

  void createAnimation();
  void resizeAnimation();
  void clearCanvas();

  QString fillModeToAspectRatio() const;

  bool runFunction(QJSValue& object, const QString& functionName,
                   const QList<QJSValue>& params);

  bool runAnimationFunction(const QString& functionName,
                            const QList<QJSValue>& params);

 private:
  QString m_source;
  bool m_readyToPlay = false;
  qreal m_speed = 1.0;
  QJSValue m_loops;
  bool m_reverse = false;
  LottieStatus m_status;
  bool m_autoPlay = false;
  QString m_fillMode = "stretch";

  QQuickItem* m_canvas = nullptr;
  QQuickItem* m_container = nullptr;

  QJSValue m_lottieModule;
  QJSValue m_lottieInstance;
  QJSValue m_animation;

  LottiePrivateWindow* m_window = nullptr;
};

#endif  // LOTTIEPRIVATE_H
