require 'xcodeproj'
project_path = 'MozillaVPN.xcodeproj'
project = Xcodeproj::Project.open(project_path)

target = project.targets.first
if target.name != 'MozillaVPN'
  puts "The first target should be MozillaVPN. Found: " + target.name
  exit 1
end

target.build_configurations.each do |config|
  config.build_settings['LD_RUNPATH_SEARCH_PATHS'] ||= '"$(inherited) @executable_path/../Frameworks"'
  config.build_settings['SWIFT_VERSION'] ||= '5.0'
  config.build_settings['CLANG_ENABLE_MODULES'] ||= 'YES'
  config.build_settings['SWIFT_OBJC_BRIDGING_HEADER'] ||= 'wireguard-apple/WireGuard/WireGuard/WireGuard-Bridging-Header.h'

  if config.name == "Release"
    config.build_settings['SWIFT_OPTIMIZATION_LEVEL'] ||= '-Onone'
  end

end

# WireGuard group
group = project.main_group.new_group("WireGuard")

[
  "wireguard-apple/WireGuard/WireGuard/Crypto/x25519.c",
  "wireguard-apple/WireGuard/WireGuard/Crypto/x25519.h",
  "wireguard-apple/WireGuard/WireGuard/ZipArchive/3rdparty/minizip/unzip.c",
  "wireguard-apple/WireGuard/WireGuard/ZipArchive/3rdparty/minizip/unzip.h",
  "macos/wireguard-go-version.h",
  "wireguard-apple/WireGuard/WireGuard/UI/macOS/View/highlighter.c",
  "wireguard-apple/WireGuard/WireGuard/UI/macOS/View/highlighter.h",
  "wireguard-apple/WireGuard/Shared/Logging/Logger.swift",
  "wireguard-apple/WireGuard/Shared/Logging/ringlogger.c",
  "wireguard-apple/WireGuard/Shared/Logging/ringlogger.h",
  "wireguard-apple/WireGuard/Shared/FileManager+Extension.swift",
  "wireguard-apple/WireGuard/Shared/Keychain.swift",
  "wireguard-apple/WireGuard/Shared/Model/key.c",
  "wireguard-apple/WireGuard/Shared/Model/key.h",
  "wireguard-apple/WireGuard/Shared/Model/Data+KeyEncoding.swift",
  "wireguard-apple/WireGuard/Shared/Model/IPAddressRange.swift",
  "wireguard-apple/WireGuard/Shared/Model/InterfaceConfiguration.swift",
  "wireguard-apple/WireGuard/Shared/Model/NETunnelProviderProtocol+Extension.swift",
  "wireguard-apple/WireGuard/Shared/Model/TunnelConfiguration.swift",
  "wireguard-apple/WireGuard/Shared/Model/TunnelConfiguration+WgQuickConfig.swift",
  "wireguard-apple/WireGuard/Shared/Model/Endpoint.swift",
  "wireguard-apple/WireGuard/Shared/Model/String+ArrayConversion.swift",
  "wireguard-apple/WireGuard/Shared/Model/PeerConfiguration.swift",
  "wireguard-apple/WireGuard/Shared/Model/DNSServer.swift",
  "wireguard-apple/WireGuard/WireGuard/LocalizationHelper.swift",
  "wireguard-apple/WireGuard/WireGuard/WireGuardAppError.swift",
  "wireguard-apple/WireGuard/WireGuard/Tunnel/ActivateOnDemandOption.swift",
  "wireguard-apple/WireGuard/WireGuard/Tunnel/TunnelsManager.swift",
  "wireguard-apple/WireGuard/WireGuard/Tunnel/TunnelErrors.swift",
  "wireguard-apple/WireGuard/WireGuard/Tunnel/TunnelStatus.swift",
  "wireguard-apple/WireGuard/WireGuard/Tunnel/TunnelConfiguration+UapiConfig.swift",
].each { |filename|
  file = group.new_file(filename)
  target.add_file_references([file])
}

# MozillaVPN group
group = project.main_group.new_group("SwiftIntegration")

[
  "macos/macoscontroller.swift",
].each { |filename|
  file = group.new_file(filename)
  target.add_file_references([file])
}

project.save
