/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import AppIntents
import NetworkExtension
import SwiftUI
import WidgetKit

struct LogoCircularAccessoryWidget: View {
  @Environment(\.colorScheme) var colorScheme
  let entry: VPNStatusEntry

  var body: some View {
    ZStack {
      if entry.isConnected {
        AccessoryWidgetBackground()
          .padding(5)
      }
      VStack(spacing: 7) {
        Image("logo")
          .resizable()
          .frame(width: 16, height: 16)
          .containerRelativeFrame(.horizontal, alignment: .center)
        Image(systemName: entry.sysImageName)
          .font(.system(size: 22))
          .containerRelativeFrame(.horizontal, alignment: .center)
      }
      .containerRelativeFrame(.vertical, alignment: .center)
    }
    .containerBackground(for: .widget) { // need this for AccessoryWidgetBackground to work, it seems
      WidgetColors.backgroundColor(colorScheme, isConnected: entry.isConnected)
    }
  }
}

struct LogoInlineAccessoryWidget: View {
  @Environment(\.colorScheme) var colorScheme
  let entry: VPNStatusEntry

  var body: some View {
    ViewThatFits(in: .horizontal) {
      HStack(spacing: 4) {
        Image(systemName: entry.sysImageName)
          .font(.system(size: 26))
        Text(entry.isConnected ? LocalizedStringResource("vpn.logoAccessoryWidget.vpnOn", defaultValue: "Mozilla VPN on") : LocalizedStringResource("vpn.logoAccessoryWidget.vpnOff", defaultValue: "Mozilla VPN off"))
      }
    }
  }
}

struct LogoAccessoryWidgetView: View {
  @Environment(\.widgetFamily) var family
  let entry: VPNStatusEntry

  var body: some View {
    switch family {
    case .accessoryCircular:
      LogoCircularAccessoryWidget(entry: entry)
    case .accessoryInline:
      LogoInlineAccessoryWidget(entry: entry)
    default:
      // This should never be seen
      Text("Error: Unsupported widget size")
    }
  }
}

struct LogoAccessoryWidget: Widget {
  static let kind = "org.mozilla.ios.FirefoxVPN.LogoAccessoryWidget"

  var body: some WidgetConfiguration {
      return StaticConfiguration(kind: Self.kind, provider: VPNStatusProvider()) { entry in
            LogoAccessoryWidgetView(entry: entry)
          }
          .configurationDisplayName(LocalizedStringResource("vpn.mobileOnboarding.panelOneTitle", defaultValue: "Mozilla VPN"))
          .description(LocalizedStringResource("vpn.logoAccessoryWidget.description", defaultValue: "See current Mozilla VPN status."))
          .supportedFamilies([.accessoryCircular, .accessoryInline])
  }
}
