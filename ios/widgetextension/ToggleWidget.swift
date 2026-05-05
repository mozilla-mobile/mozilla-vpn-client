/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import AppIntents
import NetworkExtension
import SwiftUI
import WidgetKit

struct WidgetSwitchToggleStyle: ToggleStyle {
    var onTint: Color

    func makeBody(configuration: Configuration) -> some View {
        ZStack(alignment: configuration.isOn ? .trailing : .leading) {
            Capsule()
                .fill(configuration.isOn ? onTint : Color(.systemGray4))
                .frame(width: 60, height: 36)
            Circle()
                .fill(.white)
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

struct ToggleWidgetView: View {
    let entry: VPNStatusEntry
    private static let vpnPurple = Color(red: 0.42, green: 0.20, blue: 0.89)

    var body: some View {
        VStack(alignment: .leading, spacing: 0) {
          Spacer(minLength: 0)
            HStack(spacing: 5) {
              Spacer(minLength: 0)
              Image("logo")
                      .resizable()
                      .frame(width: 35, height: 35)
              Text("Mozilla VPN")
                .font(Font.custom("MetropolisSemiBold", size: 18))
                    .multilineTextAlignment(.center)
              Spacer(minLength: 0)
            }.foregroundStyle(entry.isConnected ? .white.opacity(0.85) : Color(.secondaryLabel))

          Spacer(minLength: 20)

          HStack {
            Spacer(minLength: 0)
            Toggle("VPN", isOn: entry.isConnected, intent: ToggleIntent(value: !entry.isConnected))
              .toggleStyle(WidgetSwitchToggleStyle(onTint: entry.isConnected ? .white : Self.vpnPurple))
                   .labelsHidden()
              .labelsHidden()
              .tint(entry.isConnected ? .white : Self.vpnPurple)
            Spacer(minLength: 0)
          }
          Spacer(minLength: 0)
        }
        .containerBackground(entry.isConnected ? Self.vpnPurple : Color(.systemBackground), for: .widget)
    }
}


struct ToggleWidgetViewAlt2: View {
    let entry: VPNStatusEntry
    private static let vpnPurple = Color(red: 0.42, green: 0.20, blue: 0.89)

    var body: some View {
        VStack(alignment: .leading, spacing: 0) {
            HStack(spacing: 5) {
              Spacer(minLength: 0)
              Image("logo")
                      .resizable()
                      .frame(width: 25, height: 25)
              Spacer(minLength: 0)
            }.foregroundStyle(entry.isConnected ? .white.opacity(0.85) : Color(.secondaryLabel))


          if let exitCity = entry.exitCity, !exitCity.isEmpty {
              Spacer(minLength: 5)

              HStack {
                Spacer(minLength: 0)
                Text(((entry.entryCity?.count ?? 0) > 0) ? "\(entry.entryCity!)  \(Image(systemName: "arrow.right"))  \(exitCity)" : exitCity)
                  .font(Font.custom("Metropolis", size: 12))
                  .foregroundStyle(entry.isConnected ? .white.opacity(0.75) : Color(.secondaryLabel))
                  .multilineTextAlignment(.center)
                  .lineSpacing(3)
                Spacer(minLength: 0)
              }
            }

          Spacer(minLength: 5)

          HStack {
            Spacer(minLength: 0)
            Toggle("VPN", isOn: entry.isConnected, intent: ToggleIntent(value: !entry.isConnected))
              .toggleStyle(WidgetSwitchToggleStyle(onTint: entry.isConnected ? .white : Self.vpnPurple))
                   .labelsHidden()
              .labelsHidden()
              .tint(entry.isConnected ? .white : Self.vpnPurple)
            Spacer(minLength: 0)
          }
        }
        .containerBackground(entry.isConnected ? Self.vpnPurple : Color(.systemBackground), for: .widget)
    }
}

struct ToggleWidgetViewAlt: View {
    let entry: VPNStatusEntry
    private static let vpnPurple = Color(red: 0.42, green: 0.20, blue: 0.89)

    var body: some View {
        VStack(alignment: .leading, spacing: 0) {
            HStack(spacing: 5) {
              Spacer(minLength: 0)
              Image("logo")
                      .resizable()
                      .frame(width: 30, height: 30)
              Text("Mozilla VPN")
                    .font(Font.custom("MetropolisSemiBold", size: 14))
                    .multilineTextAlignment(.center)
                    .lineSpacing(3)
              Spacer(minLength: 0)
            }.foregroundStyle(entry.isConnected ? .white.opacity(0.85) : Color(.secondaryLabel))


          if let exitCity = entry.exitCity, !exitCity.isEmpty {
              Spacer(minLength: 5)

              HStack {
                Spacer(minLength: 0)
                Text(((entry.entryCity?.count ?? 0) > 0) ? "\(entry.entryCity!)  \(Image(systemName: "arrow.right"))  \(exitCity)" : exitCity)
                  .font(Font.custom("Metropolis", size: 14))
                  .foregroundStyle(entry.isConnected ? .white.opacity(0.75) : Color(.secondaryLabel))
                  .multilineTextAlignment(.center)
                  .lineSpacing(3)
                  .lineLimit(2)
                Spacer(minLength: 0)
              }
            }

          Spacer(minLength: 5)

          HStack {
            Spacer(minLength: 0)
            Toggle("VPN", isOn: entry.isConnected, intent: ToggleIntent(value: !entry.isConnected))
              .toggleStyle(WidgetSwitchToggleStyle(onTint: entry.isConnected ? .white : Self.vpnPurple))
                   .labelsHidden()
              .labelsHidden()
              .tint(entry.isConnected ? .white : Self.vpnPurple)
            Spacer(minLength: 0)
          }
        }
        .containerBackground(entry.isConnected ? Self.vpnPurple : Color(.systemBackground), for: .widget)
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
        .supportedFamilies([.systemSmall, .systemLarge])
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
        .supportedFamilies([.systemSmall, .systemLarge])
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
        .supportedFamilies([.systemSmall, .systemLarge])
    }
}
