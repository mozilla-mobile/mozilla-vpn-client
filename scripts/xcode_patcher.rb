require 'xcodeproj'

class XCodeprojPatcher
  attr :project
  attr :target_main

  def run(file)
    open_project file
    open_target_main

    setup_target_main
    setup_target_extension

    # TODO: let's see if we can compile the bridge here!

    @project.save
  end

  def open_project(file)
    @project = Xcodeproj::Project.open(file)
    die 'Failed to open the project file: ' + file if @project.nil?
  end

  def open_target_main
    @target_main = @project.targets.find { |target| target.to_s =='MozillaVPN' }
    die 'Unable to open MozillaVPN target' if @target_main.nil?
  end

  def setup_target_main
    @target_main.build_configurations.each do |config|
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
    group = @project.main_group.new_group('WireGuard')

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
      @target_main.add_file_references([file])
    }

    # @target_main + swift integration
    group = @project.main_group.new_group('SwiftIntegration')

    [
      'src/platforms/macos/macoscontroller.swift',
      'src/platforms/macos/macoslogger.swift',
    ].each { |filename|
      file = group.new_file(filename)
      @target_main.add_file_references([file])
    }
  end

  def setup_target_extension
    # WireGuardNetworkExtension
    target_extension = @project.new_target(:app_extension, 'WireGuardNetworkExtension', :osx)

    target_extension.build_configurations.each do |config|
      config.build_settings['LD_RUNPATH_SEARCH_PATHS'] ||= '"$(inherited) @executable_path/../Frameworks"'
      config.build_settings['SWIFT_VERSION'] ||= '5.0'
      config.build_settings['CLANG_ENABLE_MODULES'] ||= 'YES'
      config.build_settings['SWIFT_OBJC_BRIDGING_HEADER'] ||= 'macos/WireGuardNetworkExtension-Bridging-Header.h'
      config.build_settings['CODE_SIGN_ENTITLEMENTS'] ||= 'macos/MozillaVPNNetworkExtension.entitlements'
      config.build_settings['LIBRARY_SEARCH_PATHS'] ||= ['$(inherited)', 'wireguard-apple/wireguard-go-bridge/out']

      if config.name == 'Release'
        config.build_settings['SWIFT_OPTIMIZATION_LEVEL'] ||= '-Onone'
      end

    end

    group = @project.main_group.new_group('WireGuardExtension')
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
      target_extension.add_file_references([file])
    }

    # target_extension + swift integration
    group = @project.main_group.new_group('SwiftIntegration')

    [
      'src/platforms/macos/macosglue.cpp',
      'src/platforms/macos/macoslogger.swift',
    ].each { |filename|
      file = group.new_file(filename)
      target_extension.add_file_references([file])
    }

    frameworks_group = @project.groups.find { |group| group.display_name == 'Frameworks' }
    frameworks_build_phase = target_extension.build_phases.find { |build_phase| build_phase.to_s == 'FrameworksBuildPhase' }
    framework_ref = frameworks_group.new_file('libwg-go.a')
    frameworks_build_phase.add_file_reference(framework_ref)

    framework_ref = frameworks_group.new_file('Qt5Core')
    frameworks_build_phase.add_file_reference(framework_ref)

    # TODO: let's see if we can compile the bridge here!
    # This fails: @target_main.add_dependency target_extension
    container_proxy = @project.new(Xcodeproj::Project::PBXContainerItemProxy)
    container_proxy.container_portal = @project.root_object.uuid
    container_proxy.proxy_type = Xcodeproj::Constants::PROXY_TYPES[:native_target]
    container_proxy.remote_global_id_string = target_extension.uuid
    container_proxy.remote_info = target_extension.name

    dependency = @project.new(Xcodeproj::Project::PBXTargetDependency)
    dependency.name = target_extension.name
    dependency.target = @target_main
    dependency.target_proxy = container_proxy

    @target_main.dependencies << dependency
  end

  def die(msg)
   print $msg
   exit 1
  end
end

r = XCodeprojPatcher.new
r.run 'MozillaVPN.xcodeproj'
exit 0
