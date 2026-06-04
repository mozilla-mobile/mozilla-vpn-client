/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import SwiftUI

struct WidgetColors {
  static func color(_ hexCode: String) -> Color {
    let hex = hexCode.trimmingCharacters(in: CharacterSet.alphanumerics.inverted)
    var int: UInt64 = 0
    Scanner(string: hex).scanHexInt64(&int)
    let a = 255
    let r = int >> 16
    let g = int >> 8 & 0xFF
    let b = int & 0xFF
    return Color(
      .sRGB,
      red: Double(r) / 255,
      green: Double(g) / 255,
      blue: Double(b) / 255,
      opacity: Double(a) / 255
    )
  }

  static let grey3 = "FBFBFE"
  static let grey5 = "F9F9FA"
  static let grey10 = "E7E7E7"
  static let grey20 = "CECECF"
  static let grey30 = "9E9E9E"
  static let grey40 = "6D6D6E"
  static let grey45 = "42414D"
  static let grey50 = "3D3D3D"
  static let green50 = "3FE1B0"
  static let green70 = "1CC4A0"
  static let green90 = "00736C"
  static let blue20 = "00DDFF"
  static let black = "000000"
  static let white = "FFFFFF"
  static let purple90 = "321C64"
  static let brightSlate = "2B2A33"

  static func backgroundColor(_ colorScheme: ColorScheme, isConnected: Bool) -> Color {
    if isConnected {
      return colorScheme == .light ? WidgetColors.color(purple90) : WidgetColors.color(purple90)
    } else {
      return colorScheme == .light ? WidgetColors.color(grey10) : WidgetColors.color(grey45)
    }
  }

  static func logoColor(_ colorScheme: ColorScheme, isConnected: Bool) -> Color {
    if isConnected {
      return colorScheme == .light ? WidgetColors.color(white) : WidgetColors.color(white)
    } else {
      return colorScheme == .light ? WidgetColors.color(black) : WidgetColors.color(white)
    }
  }

  static func cityTextColor(_ colorScheme: ColorScheme, isConnected: Bool) -> Color {
    if isConnected {
      return colorScheme == .light ? WidgetColors.color(grey20) : WidgetColors.color(grey3)
    } else {
      return colorScheme == .light ? WidgetColors.color(grey50) : WidgetColors.color(grey3)
    }
  }

  static func toggleBackground(_ colorScheme: ColorScheme, isConnected: Bool) -> Color {
    if isConnected {
      return colorScheme == .light ? WidgetColors.color(green50) : WidgetColors.color(green70)
    } else {
      return colorScheme == .light ? WidgetColors.color(grey30) : WidgetColors.color(brightSlate)
    }
  }

  static func toggleCircle(_ colorScheme: ColorScheme, isConnected: Bool) -> Color {
    if isConnected {
      return colorScheme == .light ? WidgetColors.color(white) : WidgetColors.color(grey20)
    } else {
      return colorScheme == .light ? WidgetColors.color(white) : WidgetColors.color(grey20)
    }
  }
}
