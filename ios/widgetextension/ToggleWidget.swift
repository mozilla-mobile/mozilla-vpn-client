/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import AppIntents
import NetworkExtension
import SwiftUI
import WidgetKit

struct VPNStatusEntry: TimelineEntry {
  let date: Date
  let isConnected: Bool
  let entryCity: String?
  let exitCity: String?

  var sysImageName: String {
    return isConnected ? "shield.lefthalf.filled" : "shield.lefthalf.filled.slash"
  }
}

struct VPNStatusProvider: TimelineProvider {
  func placeholder(in context: Context) -> VPNStatusEntry {
    VPNStatusEntry(date: Date(), isConnected: true, entryCity: "Portland", exitCity: nil)
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

struct SmallToggleWidget: View {
  @Environment(\.colorScheme) var colorScheme
  let entry: VPNStatusEntry

  var body: some View {
  VStack(alignment: .leading, spacing: 0) {
    Image("logo")
      .resizable()
      .frame(width: 30, height: 30)
      .frame(maxWidth: .infinity, alignment: .center)
      .foregroundStyle(WidgetColors.logoColor(colorScheme, isConnected: entry.isConnected))
    Spacer(minLength: 10)
    CityTextToggleComponentView(entry: entry)
  }
  .containerBackground(for: .widget) {
    WidgetColors.backgroundColor(colorScheme, isConnected: entry.isConnected)
  }
}
}

struct MediumToggleWidget: View {
  @Environment(\.colorScheme) var colorScheme
  let entry: VPNStatusEntry

  var body: some View {
    ZStack {
      Image("logo")
        .resizable()
        .frame(width: 75, height: 75)
        .padding(.leading, 30)
        .containerRelativeFrame(.horizontal, alignment: .leading)
        .containerRelativeFrame(.vertical, alignment: .center)
        .foregroundStyle(WidgetColors.logoColor(colorScheme, isConnected: entry.isConnected))
      VStack {
        Spacer()
        CityTextToggleComponentView(entry: entry, largeText: true)
        Spacer()
      }
      .padding(20)
      .padding(.leading, 95)
      }
    .containerBackground(for: .widget) {
        WidgetColors.backgroundColor(colorScheme, isConnected: entry.isConnected)
    }
  }
}

struct CityTextToggleComponentView: View {
  @Environment(\.colorScheme) var colorScheme
  let entry: VPNStatusEntry
  let largeText: Bool

  init(entry: VPNStatusEntry, largeText: Bool = false) {
    self.entry = entry
    self.largeText = largeText
  }

  @ViewBuilder
  var body: some View {

    CityTextComponentView(entry: entry, largeText: largeText)

    Spacer(minLength: 10)

    Toggle("VPN", isOn: entry.isConnected, intent: ToggleIntent(value: !entry.isConnected))
      .toggleStyle(WidgetSwitchToggleStyle(capsuleColor: WidgetColors.toggleBackground(colorScheme, isConnected: entry.isConnected), circleColor: WidgetColors.toggleCircle(colorScheme, isConnected: entry.isConnected)))
      .labelsHidden()
      .frame(maxWidth: .infinity, alignment: .center)
  }
}

struct CityTextComponentView: View {
  @Environment(\.colorScheme) var colorScheme
  let entry: VPNStatusEntry
  let largeText: Bool

  init(entry: VPNStatusEntry, largeText: Bool = false) {
    self.entry = entry
    self.largeText = largeText
  }

  @ViewBuilder
  var body: some View {
    if let exitCity = entry.exitCity, !exitCity.isEmpty {

      if let entryCity = entry.entryCity, !entryCity.isEmpty {
        VStack(alignment: .leading, spacing: 5) {
          HStack {
            Text(entryCity)
            Image(systemName: "arrow.right")
          }.frame(maxWidth: .infinity, alignment: .center)
          Text(exitCity)
            .frame(maxWidth: .infinity, alignment: .center)
        }.font(Font.custom("Metropolis", size: largeText ? 20 : 14))
          .foregroundStyle(WidgetColors.cityTextColor(colorScheme, isConnected: entry.isConnected))
          .multilineTextAlignment(.center)
          .lineLimit(1)
      } else {
        Text(exitCity)
          .frame(maxWidth: .infinity, alignment: .center)
          .font(Font.custom("Metropolis", size: largeText ? 25 : 16))
          .foregroundStyle(WidgetColors.cityTextColor(colorScheme, isConnected: entry.isConnected))
          .multilineTextAlignment(.center)
          .lineSpacing(5.0)
      }
    } else {
      EmptyView()
    }
  }
}

struct WidgetSwitchToggleStyle: ToggleStyle {
  @Environment(\.widgetRenderingMode) var widgetRenderingMode
  var capsuleColor: Color
  var circleColor: Color

  func makeBody(configuration: Configuration) -> some View {
      if widgetRenderingMode != .fullColor {
          switchShape(configuration: configuration).luminanceToAlpha()
      } else {
          switchShape(configuration: configuration)
      }
  }

  private func switchShape(configuration: Configuration) -> some View {
    ZStack(alignment: configuration.isOn ? .trailing : .leading) {
      Capsule()
        .fill(widgetRenderingMode == .fullColor ? capsuleColor : WidgetColors.color(WidgetColors.grey50))
        .frame(width: 60, height: 36)
      Circle()
        .fill(circleColor)
        .shadow(color: .black.opacity(0.2), radius: 2, x: 0, y: 1)
        .frame(width: 30, height: 30)
        .padding(.horizontal, 3)
    }
  }
}

struct ToggleWidgetView: View {
  @Environment(\.widgetFamily) var family
  let entry: VPNStatusEntry

  var body: some View {
    switch family {
    case .systemSmall:
      SmallToggleWidget(entry: entry)
    case .systemMedium:
      MediumToggleWidget(entry: entry)
    default:
      // This should never be seen
      Text("Error: Unsupported widget size")
    }
  }
}

struct ToggleWidget: Widget {
  static let kind = "org.mozilla.ios.FirefoxVPN.ToggleWidget"

  var body: some WidgetConfiguration {
      let config = StaticConfiguration(kind: Self.kind, provider: VPNStatusProvider()) { entry in
            ToggleWidgetView(entry: entry)
          }
          .configurationDisplayName(LocalizedStringResource("vpn.mobileOnboarding.panelOneTitle", defaultValue: "Mozilla VPN"))
          .description(LocalizedStringResource("vpn.toggleWidget.description", defaultValue: "Turn Mozilla VPN on and off, and see the current location."))
          .supportedFamilies([.systemSmall, .systemMedium])
    if #available(iOS 26.0, *) {
      return config.disfavoredLocations([.carPlay, .iPhoneWidgetsOnMac, .standBy], for: [.systemSmall, .systemMedium])
    } else {
      return config.disfavoredLocations([.iPhoneWidgetsOnMac, .standBy], for: [.systemSmall, .systemMedium])
    }
  }
}
