/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <emscripten/bind.h>
#include <emscripten/emscripten.h>

#include <QFontDatabase>

#include "logger.h"

namespace {
Logger logger("WasmFontLoader");
}  // namespace

EMSCRIPTEN_KEEPALIVE void mzLoadFont(emscripten::val fontName,
                                     emscripten::val buffer) {
  std::string fontNameStr = fontName.as<std::string>();
  std::string bufferStr = buffer.as<std::string>();

  int id = QFontDatabase::addApplicationFontFromData(
      QByteArray(bufferStr.c_str(), bufferStr.length()));
  logger.debug() << "Loading font:"
                 << QByteArray(fontNameStr.c_str(), fontNameStr.length()) << id;
}

EMSCRIPTEN_BINDINGS(MZFontLoader) {
  emscripten::function("mzLoadFont", &mzLoadFont);
}
