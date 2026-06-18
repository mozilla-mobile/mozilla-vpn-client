/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import AppIntents
import NetworkExtension
import SwiftUI
import WidgetKit

struct CityInlineAccessoryWidget: View {
  @Environment(\.colorScheme) var colorScheme
  let entry: VPNStatusEntry

  var body: some View {
    ViewThatFits(in: .horizontal) {
      HStack(spacing: 4) {
        Image(systemName: entry.sysImageName)
          .font(.system(size: 26))
        if let exitCity = entry.exitCity, !exitCity.isEmpty {
          if let entryCity = entry.entryCity, !entryCity.isEmpty {
            Text(LocalizedStringResource("vpn.multiHopFeature.multiHopToggleCTA", defaultValue: "Multi-hop"))
          } else {
            Text(exitCity)
          }
        }
      }
    }
  }
}

struct CityRectangularAccessoryWidget: View {
  @Environment(\.colorScheme) var colorScheme
  let entry: VPNStatusEntry

  var body: some View {
    ZStack {
      if entry.isConnected {
        AccessoryWidgetBackground()
      }
      HStack {
        VStack {
          Image("logo")
            .resizable()
            .frame(width: 22, height: 22)
          Spacer()
          Image(systemName: entry.sysImageName)
            .font(.system(size: 22))
        }
        Spacer()
        CityTextComponentView(entry: entry)
          .allowsTightening(true)
          .minimumScaleFactor(0.5)
        }
        .padding(10)
    }
    .containerBackground(for: .widget) { // need this for AccessoryWidgetBackground to work, it seems
      WidgetColors.backgroundColor(colorScheme, isConnected: entry.isConnected)
    }
    .containerRelativeFrame([.horizontal, .vertical])
    .mask {
      RoundedRectangle(cornerRadius: 10, style: .continuous)
    }
  }
}

struct CityCircularAccessoryWidget: View {
  @Environment(\.colorScheme) var colorScheme
  let entry: VPNStatusEntry

  var body: some View {
    ZStack {
      if entry.isConnected {
        AccessoryWidgetBackground()
          .padding(5)
      }
      VStack(spacing: 7) {
        Image(systemName: entry.sysImageName)
          .font(.system(size: 20))
        if let exitCity = entry.exitCity, !exitCity.isEmpty {
          if let entryCity = entry.entryCity, !entryCity.isEmpty {
            formattedText(String(localized: LocalizedStringResource("vpn.multiHopFeature.multiHopToggleCTA", defaultValue: "Multi-hop")))
          } else {
            formattedText(exitCity)
          }
        }
      }
      .padding(12)
      .containerRelativeFrame([.vertical, .horizontal])
    }
    .containerBackground(for: .widget) { // need this for AccessoryWidgetBackground to work, it seems
      WidgetColors.backgroundColor(colorScheme, isConnected: entry.isConnected)
    }
  }

  func formattedText(_ text: String) -> some View {
    return Text(text)
      .font(Font.custom("Metropolis", size: 13))
      .allowsTightening(true)
      .minimumScaleFactor(0.5)
      .multilineTextAlignment(.center)
      .foregroundStyle(WidgetColors.cityTextColor(colorScheme, isConnected: entry.isConnected))
  }
}

struct CityAccessoryWidgetView: View {
  @Environment(\.widgetFamily) var family
  let entry: VPNStatusEntry

  var body: some View {
    switch family {
    case .accessoryCircular:
      CityCircularAccessoryWidget(entry: entry)
    case .accessoryInline:
      CityInlineAccessoryWidget(entry: entry)
    case .accessoryRectangular:
      CityRectangularAccessoryWidget(entry: entry)
    default:
      // This should never be seen
      Text("Error: Unsupported widget size")
    }
  }
}

struct CityAccessoryWidget: Widget {
  static let kind = "org.mozilla.ios.FirefoxVPN.CityAccessoryWidget"

  var body: some WidgetConfiguration {
      return StaticConfiguration(kind: Self.kind, provider: VPNStatusProvider()) { entry in
            CityAccessoryWidgetView(entry: entry)
          }
          .configurationDisplayName(LocalizedStringResource("vpn.mobileOnboarding.panelOneTitle", defaultValue: "Mozilla VPN"))
          .description(LocalizedStringResource("vpn.cityAccessoryWidget.description", defaultValue: "See current Mozilla VPN status, along with the current server."))
          .supportedFamilies([.accessoryCircular, .accessoryInline, .accessoryRectangular])
  }
}
