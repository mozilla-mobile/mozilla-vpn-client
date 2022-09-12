/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "composerblockbutton.h"
#include "addons/addon.h"
#include "addons/addonapi.h"
#include "leakdetector.h"
#include "logger.h"
#include "qmlengineholder.h"

#include <QDir>
#include <QJsonObject>
#include <QFile>
#include <QFileInfo>
#include <QQmlEngine>

namespace {
Logger logger(LOG_MAIN, "ComposerBlockButton");
}

// static
ComposerBlock* ComposerBlockButton::create(Composer* composer, Addon* addon,
                                           const QString& blockId,
                                           const QString& prefix,
                                           const QJsonObject& json) {
  QString javascript = json["javascript"].toString();
  if (javascript.isEmpty()) {
    logger.error() << "No javascript property for button" << blockId;
    return nullptr;
  }

  QFileInfo manifestFileInfo(addon->manifestFileName());
  QDir addonPath = manifestFileInfo.dir();

  QFile file(addonPath.filePath(javascript));
  if (!file.open(QIODevice::ReadOnly)) {
    logger.debug() << "Unable to open the javascript file" << javascript
                   << "for button" << blockId;
    return nullptr;
  }

  QJSValue function =
      QmlEngineHolder::instance()->engine()->evaluate(file.readAll());
  if (function.isError()) {
    logger.debug() << "Execution throws an error:" << function.toString();
    return nullptr;
  }

  if (!function.isCallable()) {
    logger.debug() << "The button js script should expose a callable function"
                   << blockId;
    return nullptr;
  }

  QString styleStr = json["style"].toString();
  Style style = Primary;
  if (styleStr.isEmpty() || styleStr == "primary") {  // Nothing to do.
  } else if (styleStr == "destructive") {
    style = Destructive;
  } else if (styleStr == "link") {
    style = Link;
  } else {
    logger.error() << "Unsupported button type" << styleStr;
    return nullptr;
  }

  ComposerBlockButton* block =
      new ComposerBlockButton(composer, addon, blockId, style, function);

  block->m_text.initialize(QString("%1.block.%2").arg(prefix).arg(blockId),
                           json["content"].toString());

  return block;
}

ComposerBlockButton::ComposerBlockButton(Composer* composer, Addon* addon,
                                         const QString& blockId, Style style,
                                         const QJSValue& function)
    : ComposerBlock(composer, blockId, "button"),
      m_addon(addon),
      m_style(style),
      m_function(function) {
  MVPN_COUNT_CTOR(ComposerBlockButton);
}

ComposerBlockButton::~ComposerBlockButton() {
  MVPN_COUNT_DTOR(ComposerBlockButton);
}

void ComposerBlockButton::click() const {
  QJSEngine* engine = QmlEngineHolder::instance()->engine();
  QJSValue api = engine->newQObject(m_addon->api());

  QJSValue output = m_function.call(QJSValueList{api});
  if (output.isError()) {
    logger.debug() << "Execution of the button function failed"
                   << output.toString();
  }
}

bool ComposerBlockButton::contains(const QString& string) const {
  return m_text.get().contains(string, Qt::CaseInsensitive);
}

void ComposerBlockButton::setStyle(Style style) {
  m_style = style;
  emit styleChanged();
}
