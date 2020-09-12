require 'xcodeproj'
project_path = 'MozillaVPN.xcodeproj'
project = Xcodeproj::Project.open(project_path)

targetMain = project.targets.first
if targetMain.name != 'MozillaVPN'
  puts 'The first target should be MozillaVPN. Found: ' + targetMain.name
  exit 1
end

targetMain.build_configurations.each do |config|
  config.build_settings['LD_RUNPATH_SEARCH_PATHS'] ||= '"$(inherited) @executable_path/../Frameworks"'
  config.build_settings['SWIFT_VERSION'] ||= '5.0'
  config.build_settings['CLANG_ENABLE_MODULES'] ||= 'YES'
  config.build_settings['SWIFT_OBJC_BRIDGING_HEADER'] ||= 'macos/WireGuard-Bridging-Header.h'
  config.build_settings['CODE_SIGN_ENTITLEMENTS'] ||= 'macos/MozillaVPN.entitlements'

  if config.name == 'Release'
    config.build_settings['SWIFT_OPTIMIZATION_LEVEL'] ||= '-Onone'
  end

end

# WireGuard group
group = project.main_group.new_group('WireGuard')

[
  'macos/wireguard-go-version.h',
  'wireguard-apple/WireGuard/Shared/Keychain.swift',
  'wireguard-apple/WireGuard/Shared/Model/Data+KeyEncoding.swift',
  'wireguard-apple/WireGuard/Shared/Model/IPAddressRange.swift',
  'wireguard-apple/WireGuard/Shared/Model/InterfaceConfiguration.swift',
  'wireguard-apple/WireGuard/Shared/Model/NETunnelProviderProtocol+Extension.swift',
  'wireguard-apple/WireGuard/Shared/Model/TunnelConfiguration.swift',
  'wireguard-apple/WireGuard/Shared/Model/TunnelConfiguration+WgQuickConfig.swift',
  'wireguard-apple/WireGuard/Shared/Model/Endpoint.swift',
  'wireguard-apple/WireGuard/Shared/Model/String+ArrayConversion.swift',
  'wireguard-apple/WireGuard/Shared/Model/PeerConfiguration.swift',
  'wireguard-apple/WireGuard/Shared/Model/DNSServer.swift',
  'wireguard-apple/WireGuard/WireGuard/LocalizationHelper.swift',
].each { |filename|
  file = group.new_file(filename)
  targetMain.add_file_references([file])
}

# targetMain + swift integration
group = project.main_group.new_group('SwiftIntegration')

[
  'src/platforms/macos/macoscontroller.swift',
  'src/platforms/macos/macoslogger.swift',
].each { |filename|
  file = group.new_file(filename)
  targetMain.add_file_references([file])
}

# WireGuardNetworkExtension
targetExtension = project.new_target(:app_extension, 'WireGuardNetworkExtension', :osx)
targetExtension.build_configurations.each do |config|
  config.build_settings['LD_RUNPATH_SEARCH_PATHS'] ||= '"$(inherited) @executable_path/../Frameworks"'
  config.build_settings['SWIFT_VERSION'] ||= '5.0'
  config.build_settings['CLANG_ENABLE_MODULES'] ||= 'YES'
  config.build_settings['SWIFT_OBJC_BRIDGING_HEADER'] ||= 'macos/WireGuardNetworkExtension-Bridging-Header.h'
  config.build_settings['CODE_SIGN_ENTITLEMENTS'] ||= 'macos/MozillaVPN.entitlements'

  if config.name == 'Release'
    config.build_settings['SWIFT_OPTIMIZATION_LEVEL'] ||= '-Onone'
  end

end

group = project.main_group.new_group('WireGuardExtension')
[
  'wireguard-apple/WireGuard/WireGuardNetworkExtension/PacketTunnelProvider.swift',
  'wireguard-apple/WireGuard/WireGuardNetworkExtension/PacketTunnelSettingsGenerator.swift',
  'wireguard-apple/WireGuard/WireGuardNetworkExtension/DNSResolver.swift',
  'wireguard-apple/WireGuard/WireGuardNetworkExtension/ErrorNotifier.swift',
  'wireguard-apple/WireGuard/Shared/Keychain.swift',
  'wireguard-apple/WireGuard/Shared/Model/TunnelConfiguration+WgQuickConfig.swift',
  'wireguard-apple/WireGuard/Shared/Model/NETunnelProviderProtocol+Extension.swift',
  'wireguard-apple/WireGuard/Shared/Model/String+ArrayConversion.swift',
  'wireguard-apple/WireGuard/Shared/Model/TunnelConfiguration.swift',
  'wireguard-apple/WireGuard/Shared/Model/Data+KeyEncoding.swift',
  'wireguard-apple/WireGuard/Shared/Model/IPAddressRange.swift',
  'wireguard-apple/WireGuard/Shared/Model/Endpoint.swift',
  'wireguard-apple/WireGuard/Shared/Model/DNSServer.swift',
  'wireguard-apple/WireGuard/Shared/Model/InterfaceConfiguration.swift',
  'wireguard-apple/WireGuard/Shared/Model/PeerConfiguration.swift',
  'wireguard-apple/WireGuard/Shared/FileManager+Extension.swift',
].each { |filename|
  file = group.new_file(filename)
  targetExtension.add_file_references([file])
}

# targetExtension + swift integration
group = project.main_group.new_group('SwiftIntegration')

[
  'src/platforms/macos/macosglue.cpp',
  'src/platforms/macos/macoslogger.swift',
].each { |filename|
  file = group.new_file(filename)
  targetExtension.add_file_references([file])
}

project.save
