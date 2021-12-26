/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "lottieprivate.h"
#include "lottieprivatedocument.h"
#include "lottieprivatenavigator.h"
#include "lottieprivatewindow.h"
#include "lottiestatus.h"

#include <QJSEngine>
#include <QTemporaryDir>

constexpr const char* FILLMODE_STRETCH = "stretch";
constexpr const char* FILLMODE_PAD = "pad";
constexpr const char* FILLMODE_PRESERVEASPECTFIT = "preserveAspectFit";
constexpr const char* FILLMODE_PRESERVEASPECTCROP = "preserveAspectCrop";

namespace {
static QJSEngine* s_engine = nullptr;
static QString s_userAgent;
}  // namespace

// static
void LottiePrivate::initialize(QJSEngine* engine, const QString& userAgent) {
  Q_ASSERT(engine);
  s_engine = engine;

  Q_ASSERT(!userAgent.isEmpty());
  s_userAgent = userAgent;
}

// static
QJSEngine* LottiePrivate::engine() {
  Q_ASSERT(s_engine);
  return s_engine;
}

// static
const QString& LottiePrivate::userAgent() { return s_userAgent; }

LottiePrivate::LottiePrivate(QQuickItem* parent)
    : QQuickItem(parent), m_loops(false) {}

void LottiePrivate::setSource(const QString& source) {
  m_source = source;
  emit sourceChanged();
  destroyAndRecreate();
}

void LottiePrivate::setReadyToPlay(bool readyToPlay) {
  m_readyToPlay = readyToPlay;
  emit readyToPlayChanged();
  createAnimation();
}

void LottiePrivate::createAnimation() {
  if (!m_readyToPlay || !m_canvas || m_source.isEmpty()) return;

  if (!m_lottieModule.isObject()) {
    QByteArray jsModule;
    {
      QFile js(":/lottie/lottie/lottie_wrap.js.template");
      if (!js.open(QFile::ReadOnly)) {
        m_status.error("Unable to open the template resource.");
        return;
      }
      jsModule.append(js.readAll());
    }
    {
      QFile js(":/lottie/lottie/lottie.min.js");
      if (!js.open(QFile::ReadOnly)) {
        m_status.error("Unable to open the lottie resource.");
        return;
      }
      jsModule.replace("__LOTTIE__", js.readAll());
    }

    QTemporaryDir tmpDir;
    if (!tmpDir.isValid()) {
      m_status.error("Failed to create a temporary folder.");
      return;
    }

    QFile modulePath(QDir(tmpDir.path()).filePath("lottie.js"));
    if (!modulePath.open(QFile::WriteOnly)) {
      m_status.error("Failed to load the generated lottie.js.");
      return;
    }

    modulePath.write(jsModule);

    m_lottieModule =
        engine()->importModule(QFileInfo(modulePath).absoluteFilePath());
    if (m_lottieModule.isError()) {
      QString errorMessage("Exception processing the lottie js: ");
      errorMessage.append(m_lottieModule.toString());
      m_status.error(errorMessage);
      return;
    }

    Q_ASSERT(m_lottieModule.hasProperty("initialize"));
    QJSValue initialize = m_lottieModule.property("initialize");
    Q_ASSERT(initialize.isCallable());

    QJSValue windowObj = createWindowObject();
    QJSValue navigatorObj = createNavigatorObject();
    QJSValue documentObj = createDocumentObject();

    QJSValue ret = initialize.callWithInstance(
        m_lottieModule, QList<QJSValue>{windowObj, documentObj, navigatorObj});
    if (ret.isError()) {
      QString errorMessage("Failed to initialize the lottie module: ");
      errorMessage.append(ret.toString());
      m_status.error(errorMessage);
      return;
    }
  }

  Q_ASSERT(m_lottieInstance.isObject());

  Q_ASSERT(m_lottieInstance.hasProperty("loadAnimation"));
  QJSValue loadAnimation = m_lottieInstance.property("loadAnimation");
  Q_ASSERT(loadAnimation.isCallable());

  QJSValue jsonParser =
      engine()->globalObject().property("JSON").property("parse");
  Q_ASSERT(jsonParser.isCallable());

  QFile file(m_source);
  if (!file.open(QFile::ReadOnly)) {
    QString errorMessage("Failed to open the source URL ");
    errorMessage.append(m_source);
    m_status.error(errorMessage);
    return;
  }

  QJSValue jsonData =
      jsonParser.call(QList<QJSValue>{QJSValue(QString(file.readAll()))});
  if (jsonData.isError()) {
    QString errorMessage("Failed to parse the source as JSON: ");
    errorMessage.append(jsonData.toString());
    m_status.error(errorMessage);
    return;
  }

  QJSValue containerValue = engine()->toScriptValue(m_container);

  QJSValue rendererSettings = engine()->newObject();
  rendererSettings.setProperty("clearCanvas", true);
  rendererSettings.setProperty("preserveAspectRatio", fillModeToAspectRatio());

  QJSValue obj = engine()->newObject();
  obj.setProperty("container", engine()->toScriptValue(m_container));
  obj.setProperty("renderer", "canvas");
  obj.setProperty("rendererSettings", rendererSettings);
  obj.setProperty("loop", m_loops);
  obj.setProperty("autoplay", m_autoPlay);
  obj.setProperty("animationData", jsonData);

  QJSValue animation =
      loadAnimation.callWithInstance(m_lottieInstance, QList<QJSValue>{obj});
  if (animation.isError()) {
    QString errorMessage("Failed to initialize the lottie component: ");
    errorMessage.append(animation.toString());
    errorMessage.append(" - line: ");
    errorMessage.append(
        QString::number(animation.property("lineNumber").toInt()));
    m_status.error(errorMessage);
    return;
  }

  destroyAnimation();
  m_animation = animation;

  Q_ASSERT(m_lottieModule.hasProperty("addListeners"));
  QJSValue addListeners = m_lottieModule.property("addListeners");
  addListeners.callWithInstance(
      m_lottieModule,
      QList<QJSValue>{engine()->toScriptValue(this), animation});

  applySpeed();
  applyDirection();
}

void LottiePrivate::setSpeed(qreal speed) {
  m_speed = speed;
  emit speedChanged();
  applySpeed();
}

void LottiePrivate::setLoops(QJSValue loops) {
  if (!loops.isBool() && !loops.isNumber()) {
    return;
  }

  m_loops = loops;
  emit loopsChanged();
  destroyAndRecreate();
}

void LottiePrivate::setReverse(bool reverse) {
  m_reverse = reverse;
  emit reverseChanged();
  applyDirection();
}

void LottiePrivate::setAutoPlay(bool autoPlay) {
  m_autoPlay = autoPlay;
  emit autoPlayChanged();
  destroyAndRecreate();
}

void LottiePrivate::setFillMode(const QString& fillMode) {
  if (fillMode != FILLMODE_STRETCH && fillMode != FILLMODE_PRESERVEASPECTFIT &&
      fillMode != FILLMODE_PRESERVEASPECTCROP && fillMode != FILLMODE_PAD)
    return;

  m_fillMode = fillMode;
  emit fillModeChanged();
  destroyAndRecreate();
}

QJSValue LottiePrivate::createWindowObject() {
  return engine()->toScriptValue(new LottiePrivateWindow(this));
}

QJSValue LottiePrivate::createNavigatorObject() {
  return engine()->toScriptValue(new LottiePrivateNavigator(this));
}

QJSValue LottiePrivate::createDocumentObject() {
  return engine()->toScriptValue(new LottiePrivateDocument(this));
}

bool LottiePrivate::runFunction(QJSValue& object, const QString& functionName,
                                const QList<QJSValue>& params) {
  if (object.isObject() && object.hasProperty(functionName)) {
    QJSValue func = object.property(functionName);
    Q_ASSERT(func.isCallable());
    func.callWithInstance(object, params);
    return true;
  }

  return false;
}

bool LottiePrivate::runAnimationFunction(const QString& functionName,
                                         const QList<QJSValue>& params) {
  return runFunction(m_animation, functionName, params);
}

void LottiePrivate::applySpeed() {
  runAnimationFunction("setSpeed", QList<QJSValue>{m_speed});
}

void LottiePrivate::applyDirection() {
  runAnimationFunction("setDirection", QList<QJSValue>{m_reverse ? -1 : 1});
}

void LottiePrivate::destroyAnimation() {
  runAnimationFunction("destroy", QList<QJSValue>());
  m_animation = QJSValue();
}

void LottiePrivate::clearAndResize() {
  clearCanvas();
  resizeAnimation();
}

void LottiePrivate::clearCanvas() {
  QJSValue canvasValue = engine()->toScriptValue(m_canvas);

  if (!canvasValue.isObject() || !canvasValue.hasProperty("available")) {
    return;
  }

  QJSValue available = canvasValue.property("available");
  Q_ASSERT(available.isBool());
  if (!available.toBool()) {
    return;
  }

  QJSValue getContext = canvasValue.property("getContext");
  Q_ASSERT(getContext.isCallable());
  QJSValue ctx =
      getContext.callWithInstance(canvasValue, QList<QJSValue>{"2d"});
  if (ctx.isObject() && ctx.hasProperty("reset")) {
    QJSValue ctxReset = ctx.property("reset");
    Q_ASSERT(ctxReset.isCallable());
    QJSValue ctx = ctxReset.callWithInstance(canvasValue, QList<QJSValue>());
  }

  runFunction(canvasValue, "requestPaint", QList<QJSValue>());
}

void LottiePrivate::resizeAnimation() {
  runAnimationFunction("resize", QList<QJSValue>());
}

void LottiePrivate::destroyAndRecreate() {
  bool wasPlaying = m_status.playing();

  destroyAnimation();

  QJSValue containerValue = engine()->toScriptValue(m_container);
  runFunction(containerValue, "clear", QList<QJSValue>());

  createAnimation();

  if (wasPlaying) play();
}

void LottiePrivate::play() {
  if (runAnimationFunction("play", QList<QJSValue>())) {
    m_status.update(true);
  }
}

void LottiePrivate::pause() {
  if (runAnimationFunction("pause", QList<QJSValue>())) {
    m_status.update(false);
  }
}

void LottiePrivate::stop() {
  if (runAnimationFunction("stop", QList<QJSValue>())) {
    m_status.reset();
  }
}

QString LottiePrivate::fillModeToAspectRatio() const {
  if (m_fillMode == FILLMODE_PAD) return "orig";

  if (m_fillMode == FILLMODE_PRESERVEASPECTFIT) return "xMidYMid meet";

  if (m_fillMode == FILLMODE_PRESERVEASPECTCROP) return "xMidYMid slice";

  return "none";
}

void LottiePrivate::eventPlayingCompleted() { m_status.reset(); }

void LottiePrivate::eventLoopCompleted() { emit loopCompleted(); }

void LottiePrivate::eventEnterFrame(const QJSValue& value) {
  m_status.update(true, value.property("currentTime").toNumber(),
                  value.property("totalTime").toInt());
}

QJSValue LottiePrivate::status() { return engine()->toScriptValue(&m_status); }
