/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import AppIntents
import NetworkExtension
import SwiftUI
import WidgetKit

struct WidgetSwitchToggleStyle: ToggleStyle {
  var capsuleColor: Color
  var circleColor: Color

  func makeBody(configuration: Configuration) -> some View {
    ZStack(alignment: configuration.isOn ? .trailing : .leading) {
      Capsule()
        .fill(capsuleColor)
        .frame(width: 60, height: 36)
      Circle()
        .fill(circleColor)
        .shadow(color: .black.opacity(0.2), radius: 2, x: 0, y: 1)
        .frame(width: 30, height: 30)
        .padding(.horizontal, 3)
    }
  }
}

struct VPNStatusEntry: TimelineEntry {
  let date: Date
  let isConnected: Bool
  let entryCity: String?
  let exitCity: String?
}

struct VPNStatusProvider: TimelineProvider {
  func placeholder(in context: Context) -> VPNStatusEntry {
    VPNStatusEntry(date: Date(), isConnected: false, entryCity: nil, exitCity: nil)
  }

  func getSnapshot(in context: Context, completion: @escaping (VPNStatusEntry) -> Void) {
    Task { completion(await currentEntry()) }
  }

  func getTimeline(in context: Context, completion: @escaping (Timeline<VPNStatusEntry>) -> Void) {
    Task {
      let entry = await currentEntry()
      let nextRefresh = Calendar.current.date(byAdding: .hour, value: 1, to: Date())!
      completion(Timeline(entries: [entry], policy: .after(nextRefresh)))
    }
  }

  private func currentEntry() async -> VPNStatusEntry {
    let logger = IOSLoggerImpl(tag: "ToggleWidget")
    do {
      guard let tunnel = try await NETunnelProviderManager.ourVpnTunnel(),
            let config = (tunnel.protocolConfiguration as? NETunnelProviderProtocol)?.providerConfiguration else {
        return VPNStatusEntry(date: Date(), isConnected: false, entryCity: nil, exitCity: nil)
      }
      let exitCity = config["exitCity"] as? String
      let entryCity = config["entryCity"] as? String
      return VPNStatusEntry(
        date: Date(),
        isConnected: tunnel.isConnected,
        entryCity: entryCity,
        exitCity: exitCity
      )
    } catch {
      logger.error(message: "Error fetching VPN status: \(error.localizedDescription)")
      return VPNStatusEntry(date: Date(), isConnected: false, entryCity: nil, exitCity: nil)
    }
  }
}

/*
 colors




 ----light mode
 ---off

 logo / logo text = black
 city text = gray
 background = background gray
 frame = purple?
 toggle background = gray
 toggle center = white

 ---on

 logo / logo text = white
 city text = almost white
 background = purple
 frame = purple?
 toggle background = green
 toggle center = white



 ----dark mode
 logo / logo text = white
 city text = almost white
 background = light-ish gray
 frame = purple? maybe black?
 toggle background = real dark
 toggle center = dark grey
 --- off

 ---on
 logo / logo text = white
 city text = almost white
 background = purple
 frame = purple?
 toggle background = shitty blue
 toggle center = dark grey

 */
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
  static let grey20 = "CECECF"
  static let grey30 = "9E9E9E"
  static let grey40 = "6D6D6E"
  static let grey45 = "42414D"
  static let green50 = "3FE1B0"
  static let blue20 = "00DDFF"
  static let black = "000000"
  static let white = "FFFFFF"
  static let purple90 = "321C64"
  static let brightSlate = "2B2A33"

  static func backgroundColor(_ colorScheme: ColorScheme, isConnected: Bool) -> Color {
    if isConnected {
      return colorScheme == .light ? WidgetColors.color(purple90) : WidgetColors.color(purple90)
    } else {
      return colorScheme == .light ? WidgetColors.color(grey5) : WidgetColors.color(grey45)
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
      return colorScheme == .light ? WidgetColors.color(grey40) : WidgetColors.color(grey3)
    }
  }

  static func frameColor(_ colorScheme: ColorScheme, isConnected: Bool) -> Color {
    if isConnected {
      return colorScheme == .light ? .red : .blue
    } else {
      return colorScheme == .light ? .red : .blue
    }
  }

  static func toggleBackground(_ colorScheme: ColorScheme, isConnected: Bool) -> Color {
    if isConnected {
      return colorScheme == .light ? WidgetColors.color(green50) : WidgetColors.color(blue20)
    } else {
      return colorScheme == .light ? WidgetColors.color(grey30) : WidgetColors.color(brightSlate)
    }
  }

  static func toggleCircle(_ colorScheme: ColorScheme, isConnected: Bool) -> Color {
    if isConnected {
      return colorScheme == .light ? WidgetColors.color(white) : WidgetColors.color(grey30)
    } else {
      return colorScheme == .light ? WidgetColors.color(white) : WidgetColors.color(grey30)
    }
  }
}

struct ToggleWidgetView: View {
  @Environment(\.colorScheme) var colorScheme
  let entry: VPNStatusEntry

  var body: some View {
    VStack(alignment: .leading, spacing: 0) {
      Spacer(minLength: 10)
      HStack(spacing: 5) {
        Image("logo")
          .resizable()
          .frame(width: 35, height: 35)
        Text("Mozilla VPN")
          .font(Font.custom("MetropolisSemiBold", size: 18))
          .multilineTextAlignment(.center)
      }.foregroundStyle(WidgetColors.logoColor(colorScheme, isConnected: entry.isConnected))
        .frame(maxWidth: .infinity, alignment: .center)

      Spacer(minLength: 20)

      Toggle("VPN", isOn: entry.isConnected, intent: ToggleIntent(value: !entry.isConnected))
        .toggleStyle(WidgetSwitchToggleStyle(capsuleColor: WidgetColors.toggleBackground(colorScheme, isConnected: entry.isConnected), circleColor: WidgetColors.toggleCircle(colorScheme, isConnected: entry.isConnected)))
        .labelsHidden()
        .frame(maxWidth: .infinity, alignment: .center)
      Spacer(minLength: 10)
    }
    .containerBackground(for: .widget) {
      ContainerRelativeShape()
        .inset(by: 10)
        .fill(WidgetColors.backgroundColor(colorScheme, isConnected: entry.isConnected))
        .background(WidgetColors.color(WidgetColors.purple90))
    }
  }
}

struct ToggleWidgetViewAlt2: View {
  @Environment(\.colorScheme) var colorScheme
  let entry: VPNStatusEntry
  private static let vpnPurple = Color(red: 0.42, green: 0.20, blue: 0.89)

  var body: some View {
    VStack(alignment: .leading, spacing: 0) {
      Spacer(minLength: 5)
      HStack(spacing: 5) {
        Spacer(minLength: 0)
        Image("logo")
          .resizable()
          .frame(width: 25, height: 25)
        Spacer(minLength: 0)
      }.foregroundStyle(WidgetColors.logoColor(colorScheme, isConnected: entry.isConnected))

      if let exitCity = entry.exitCity, !exitCity.isEmpty {
        Spacer(minLength: 10)

        if let entryCity = entry.entryCity, !entryCity.isEmpty {

          VStack(alignment: .leading, spacing: 3) {

            HStack {
              Text(entryCity)
                .layoutPriority(1)
                .lineLimit(1)
              Image(systemName: "arrow.right")
            }.frame(maxWidth: .infinity, alignment: .center)
            Text(exitCity)
              .frame(maxWidth: .infinity, alignment: .center)
              .layoutPriority(0)
              .lineLimit(1)
          }.font(Font.custom("Metropolis", size: 13))
            .foregroundStyle(WidgetColors.cityTextColor(colorScheme, isConnected: entry.isConnected))
            .multilineTextAlignment(.center)
            .lineLimit(1)

        } else {
          HStack {
            Spacer(minLength: 0)
            Text(exitCity)
            Spacer(minLength: 0)
          }.font(Font.custom("Metropolis", size: 16))
            .foregroundStyle(WidgetColors.cityTextColor(colorScheme, isConnected: entry.isConnected))
            .multilineTextAlignment(.center)
        }
      }

      Spacer(minLength: 10)

      HStack {
        Spacer(minLength: 0)
        Toggle("VPN", isOn: entry.isConnected, intent: ToggleIntent(value: !entry.isConnected))
          .toggleStyle(WidgetSwitchToggleStyle(capsuleColor: WidgetColors.toggleBackground(colorScheme, isConnected: entry.isConnected), circleColor: WidgetColors.toggleCircle(colorScheme, isConnected: entry.isConnected)))
          .labelsHidden()
          .frame(maxWidth: .infinity, alignment: .center)
        Spacer(minLength: 0)
      }
      Spacer(minLength: 5)
    }
    .containerBackground(for: .widget) {
      ContainerRelativeShape()
        .inset(by: 5)
        .fill(WidgetColors.backgroundColor(colorScheme, isConnected: entry.isConnected))
        .background(WidgetColors.color(WidgetColors.purple90))
    }
  }
}

struct ToggleWidgetViewAlt: View {
  @Environment(\.colorScheme) var colorScheme
  let entry: VPNStatusEntry
  private static let vpnPurple = Color(red: 0.42, green: 0.20, blue: 0.89)

  var body: some View {
    VStack(alignment: .center, spacing: 0) {
      HStack(spacing: 5) {
        Image("logo")
          .resizable()
          .frame(width: 30, height: 30)
        Text("Mozilla VPN")
          .font(Font.custom("MetropolisSemiBold", size: 14))
          .multilineTextAlignment(.center)
          .lineSpacing(3)
      }.foregroundStyle(WidgetColors.logoColor(colorScheme, isConnected: entry.isConnected))
        .frame(maxWidth: .infinity, alignment: .center)


      if let exitCity = entry.exitCity, !exitCity.isEmpty {
        Spacer(minLength: 10)
        VStack(alignment: .leading, spacing: 3) {

          if let entryCity = entry.entryCity, !entryCity.isEmpty {

            HStack {
              Spacer(minLength: 0)
              Text(entryCity)
              Image(systemName: "arrow.right")
              Spacer(minLength: 0)
            }
          }
          HStack {
            Spacer(minLength: 0)
            Text(exitCity)
            Spacer(minLength: 0)
          }
        }.font(Font.custom("Metropolis", size: 14))
          .foregroundStyle(WidgetColors.cityTextColor(colorScheme, isConnected: entry.isConnected))
          .multilineTextAlignment(.center)
          .lineLimit(1)
      }


      Spacer(minLength: 10)

      HStack {
        Spacer(minLength: 0)
        Toggle("VPN", isOn: entry.isConnected, intent: ToggleIntent(value: !entry.isConnected))
          .toggleStyle(WidgetSwitchToggleStyle(capsuleColor: WidgetColors.toggleBackground(colorScheme, isConnected: entry.isConnected), circleColor: WidgetColors.toggleCircle(colorScheme, isConnected: entry.isConnected)))
          .labelsHidden()
          .frame(maxWidth: .infinity, alignment: .center)
        Spacer(minLength: 0)
      }
    }
    .frame(maxHeight: .infinity, alignment: .center)
    .containerBackground(for: .widget) {
      ContainerRelativeShape()
        .inset(by: 5)
        .fill(WidgetColors.backgroundColor(colorScheme, isConnected: entry.isConnected))
        .background(WidgetColors.color(WidgetColors.purple90))
    }
  }
}

struct ToggleWidget: Widget {
  static let kind = "org.mozilla.ios.FirefoxVPN.ToggleWidget"

  var body: some WidgetConfiguration {
    StaticConfiguration(kind: Self.kind, provider: VPNStatusProvider()) { entry in
      ToggleWidgetView(entry: entry) // oh man update this
    }
    .configurationDisplayName("Mozilla VPN")
    .description("Shows your VPN connection status.") // LcalizedStringResource("vpn.statusWidget.description", defaultValue: "Shows your VPN connection status."))
    .supportedFamilies([.systemSmall])
  }
}

struct ToggleWidgetAlt: Widget {
  static let kind = "org.mozilla.ios.FirefoxVPN.ToggleWidgetAlt"

  var body: some WidgetConfiguration {
    StaticConfiguration(kind: Self.kind, provider: VPNStatusProvider()) { entry in
      ToggleWidgetViewAlt(entry: entry) // oh man update this
    }
    .configurationDisplayName("Mozilla VPN")
    .description("Shows your VPN connection status.") // LcalizedStringResource("vpn.statusWidget.description", defaultValue: "Shows your VPN connection status."))
    .supportedFamilies([.systemSmall])
  }
}

struct ToggleWidgetAlt2: Widget {
  static let kind = "org.mozilla.ios.FirefoxVPN.ToggleWidgetAlt2"

  var body: some WidgetConfiguration {
    StaticConfiguration(kind: Self.kind, provider: VPNStatusProvider()) { entry in
      ToggleWidgetViewAlt2(entry: entry) // oh man update this
    }
    .configurationDisplayName("Mozilla VPN")
    .description("Shows your VPN connection status.") // LcalizedStringResource("vpn.statusWidget.description", defaultValue: "Shows your VPN connection status."))
    .supportedFamilies([.systemSmall])
  }
}
