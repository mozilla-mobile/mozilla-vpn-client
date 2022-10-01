/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COMPOSERBLOCKBUTTON_H
#define COMPOSERBLOCKBUTTON_H

#include "../addons/addonproperty.h"
#include "composerblock.h"

#include <QJSValue>

class Addon;

class ComposerBlockButton final : public ComposerBlock {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(ComposerBlockButton)
  QML_NAMED_ELEMENT(VPNComposerBlockButton)
  QML_UNCREATABLE("")

 public:
  ADDON_PROPERTY(text, m_text, getText, setText, retranslationCompleted)
  Q_PROPERTY(Style style READ style WRITE setStyle NOTIFY styleChanged)

  enum Style { Primary, Destructive, Link };
  Q_ENUM(Style);

  static ComposerBlock* create(Composer* composer, Addon* addon,
                               const QString& blockId, const QString& prefix,
                               const QJsonObject& json);
  virtual ~ComposerBlockButton();

  Style style() const { return m_style; }
  void setStyle(Style style);

  bool contains(const QString& string) const override;

  Q_INVOKABLE void click() const;

 private:
  ComposerBlockButton(Composer* composer, Addon* addon, const QString& blockId,
                      Style style, const QJSValue& function);

 signals:
  void styleChanged();

 private:
  Addon* m_addon = nullptr;

  AddonProperty m_text;
  Style m_style;
  const QJSValue m_function;
};

#endif  // COMPOSERBLOCKBUTTON_H
