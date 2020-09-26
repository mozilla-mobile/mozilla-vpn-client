require 'xcodeproj'

class XCodeprojPatcher
  attr :project
  attr :target_main
  attr :target_extension

  def run(file, version, platform)
    open_project file
    open_target_main

    setup_target_main version, platform
    setup_target_extension version, platform

    setup_target_loginitem version if platform == "macos"

    setup_target_go

    @project.save
  end

  def open_project(file)
    @project = Xcodeproj::Project.open(file)
    die 'Failed to open the project file: ' + file if @project.nil?
  end

  def open_target_main
    @target_main = @project.targets.find { |target| target.to_s == 'MozillaVPN' }
    return @target_main if not @target_main.nil?

    @target_main = @project.targets.find { |target| target.to_s == 'MozillaVPN_ios' }
    return @target_main if not @target_main.nil?

    die 'Unable to open MozillaVPN target'
  end

  def setup_target_main(version, platform)
    @target_main.build_configurations.each do |config|
      config.build_settings['LD_RUNPATH_SEARCH_PATHS'] ||= '"$(inherited) @executable_path/../Frameworks"'
      config.build_settings['SWIFT_VERSION'] ||= '5.0'
      config.build_settings['CLANG_ENABLE_MODULES'] ||= 'YES'
      config.build_settings['SWIFT_OBJC_BRIDGING_HEADER'] ||= 'macos/app/WireGuard-Bridging-Header.h'

      # Versions and names
      config.build_settings['MARKETING_VERSION'] ||= version
      config.build_settings['CURRENT_PROJECT_VERSION'] ||= version + "." + Time.now.strftime("%Y%m%d%H%M")
      config.build_settings['PRODUCT_BUNDLE_IDENTIFIER'] = 'org.mozilla.' + platform + '.FirefoxVPN'
      config.build_settings['PRODUCT_NAME'] = 'Mozilla VPN'

      # other config
      config.build_settings['INFOPLIST_FILE'] ||= platform + '/app/Info.plist'
      config.build_settings['CODE_SIGN_ENTITLEMENTS'] ||= platform +'/app/MozillaVPN.entitlements'
      config.build_settings['CODE_SIGN_IDENTITY'] ||= 'Apple Development'
      config.build_settings['ENABLE_BITCODE'] ||= 'NO' if platform == 'ios'
      config.build_settings['SDKROOT'] = 'iphoneos' if platform == 'ios'

      if config.name == 'Release'
        config.build_settings['SWIFT_OPTIMIZATION_LEVEL'] ||= '-Onone'
      end
    end

    # WireGuard group
    group = @project.main_group.new_group('WireGuard')

    [
      'macos/gobridge/wireguard-go-version.h',
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
      'wireguard-apple/WireGuard/Shared/FileManager+Extension.swift',
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

  def setup_target_extension(version, platform)
    @target_extension = @project.new_target(:app_extension, 'WireGuardNetworkExtension', :osx)

    @target_extension.build_configurations.each do |config|
      config.build_settings['LD_RUNPATH_SEARCH_PATHS'] ||= '"$(inherited) @executable_path/../Frameworks"'
      config.build_settings['SWIFT_VERSION'] ||= '5.0'
      config.build_settings['CLANG_ENABLE_MODULES'] ||= 'YES'
      config.build_settings['SWIFT_OBJC_BRIDGING_HEADER'] ||= 'macos/networkextension/WireGuardNetworkExtension-Bridging-Header.h'
      config.build_settings['LIBRARY_SEARCH_PATHS'] ||= ['$(inherited)', 'wireguard-apple/wireguard-go-bridge/out'] if platform == 'macos'

      # Versions and names
      config.build_settings['MARKETING_VERSION'] ||= version
      config.build_settings['CURRENT_PROJECT_VERSION'] ||= version + "." + Time.now.strftime("%Y%m%d%H%M")
      config.build_settings['PRODUCT_BUNDLE_IDENTIFIER'] ||= 'org.mozilla.' + platform + '.FirefoxVPN.network-extension'
      config.build_settings['PRODUCT_NAME'] = 'WireGuardNetworkExtension'

      # other configs
      config.build_settings['INFOPLIST_FILE'] ||= 'wireguard-apple/WireGuard/WireGuardNetworkExtension/Info.plist'
      config.build_settings['CODE_SIGN_ENTITLEMENTS'] ||= platform + '/networkextension/MozillaVPNNetworkExtension.entitlements'
      config.build_settings['CODE_SIGN_IDENTITY'] = 'Apple Development'

      if platform == 'ios'
        config.build_settings['ENABLE_BITCODE'] ||= 'NO'
        config.build_settings['SDKROOT'] = 'iphoneos'

        config.build_settings['OTHER_LDFLAGS'] ||= [
          "-stdlib=libc++",
          "-Wl,-rpath,@executable_path/Frameworks",
          "-L/Users/baku/Qt/5.15.0/ios/plugins/platforms",
          "-framework",
          "AssetsLibrary",
          "-framework",
          "MobileCoreServices",
          "-lm",
          "-framework",
          "UIKit",
          "-lz",
          "-framework",
          "OpenGLES",
        ]
      end

      # This is needed to compile the macosglue without Qt.
      config.build_settings['GCC_PREPROCESSOR_DEFINITIONS'] ||= 'MACOS_EXTENSION=1'

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
      @target_extension.add_file_references([file])
    }

    # @target_extension + swift integration
    group = @project.main_group.new_group('SwiftIntegration')

    [
      'src/platforms/macos/macosglue.cpp',
      'src/platforms/macos/macoslogger.swift',
    ].each { |filename|
      file = group.new_file(filename)
      @target_extension.add_file_references([file])
    }

    frameworks_group = @project.groups.find { |group| group.display_name == 'Frameworks' }
    frameworks_build_phase = @target_extension.build_phases.find { |build_phase| build_phase.to_s == 'FrameworksBuildPhase' }

    frameworks_build_phase.clear

    framework_ref = frameworks_group.new_file('libwg-go.a')
    frameworks_build_phase.add_file_reference(framework_ref)

    framework_ref = frameworks_group.new_file('NetworkExtension.framework')
    frameworks_build_phase.add_file_reference(framework_ref)

    # This fails: @target_main.add_dependency @target_extension
    container_proxy = @project.new(Xcodeproj::Project::PBXContainerItemProxy)
    container_proxy.container_portal = @project.root_object.uuid
    container_proxy.proxy_type = Xcodeproj::Constants::PROXY_TYPES[:native_target]
    container_proxy.remote_global_id_string = @target_extension.uuid
    container_proxy.remote_info = @target_extension.name

    dependency = @project.new(Xcodeproj::Project::PBXTargetDependency)
    dependency.name = @target_extension.name
    dependency.target = @target_main
    dependency.target_proxy = container_proxy

    @target_main.dependencies << dependency

    copy_appex = @target_main.new_copy_files_build_phase
    copy_appex.name = 'Copy Network-Extension plugin'
    copy_appex.symbol_dst_subfolder_spec = :plug_ins

    appex_file = copy_appex.add_file_reference @target_extension.product_reference
    appex_file.settings = { "ATTRIBUTES" => ['RemoveHeadersOnCopy'] }
  end

  def setup_target_go
    target_go = legacy_target = @project.new(Xcodeproj::Project::PBXLegacyTarget)

    target_go.build_working_directory = 'wireguard-apple/wireguard-go-bridge'
    target_go.build_tool_path = 'make'
    target_go.pass_build_settings_in_environment = '1'
    target_go.build_arguments_string = '$(ACTION)'
    target_go.name = 'WireGuardGoBridge'
    target_go.product_name = 'WireGuardGoBridge'

    @project.targets << target_go

    @target_extension.add_dependency target_go
  end

  def setup_target_loginitem(version)
    @target_loginitem = @project.new_target(:application, 'MozillaVPNLoginItem', :osx)

    @target_loginitem.build_configurations.each do |config|
      config.build_settings['LD_RUNPATH_SEARCH_PATHS'] ||= '"$(inherited) @executable_path/../Frameworks"'

      # Versions and names
      config.build_settings['MARKETING_VERSION'] ||= version
      config.build_settings['CURRENT_PROJECT_VERSION'] ||= version + "." + Time.now.strftime("%Y%m%d%H%M")
      config.build_settings['PRODUCT_BUNDLE_IDENTIFIER'] ||= 'org.mozilla.macos.FirefoxVPN.login-item'
      config.build_settings['PRODUCT_NAME'] = 'MozillaVPNLoginItem'

      # other configs
      config.build_settings['INFOPLIST_FILE'] ||= 'macos/loginitem/Info.plist'
      config.build_settings['CODE_SIGN_ENTITLEMENTS'] ||= 'macos/loginitem/MozillaVPNLoginItem.entitlements'
      config.build_settings['CODE_SIGN_IDENTITY'] = 'Apple Development'
      config.build_settings['SKIP_INSTALL'] = 'YES'

      if config.name == 'Release'
        config.build_settings['SWIFT_OPTIMIZATION_LEVEL'] ||= '-Onone'
      end
    end

    group = @project.main_group.new_group('LoginItem')
    [
      'macos/loginitem/main.m',
    ].each { |filename|
      file = group.new_file(filename)
      @target_loginitem.add_file_references([file])
    }

    # This fails: @target_main.add_dependency @target_loginitem
    container_proxy = @project.new(Xcodeproj::Project::PBXContainerItemProxy)
    container_proxy.container_portal = @project.root_object.uuid
    container_proxy.proxy_type = Xcodeproj::Constants::PROXY_TYPES[:native_target]
    container_proxy.remote_global_id_string = @target_loginitem.uuid
    container_proxy.remote_info = @target_loginitem.name

    dependency = @project.new(Xcodeproj::Project::PBXTargetDependency)
    dependency.name = @target_loginitem.name
    dependency.target = @target_main
    dependency.target_proxy = container_proxy

    @target_main.dependencies << dependency

    copy_app = @target_main.new_copy_files_build_phase
    copy_app.name = 'Copy LoginItem'
    copy_app.symbol_dst_subfolder_spec = :wrapper
    copy_app.dst_path = 'Contents/Library/LoginItems'

    app_file = copy_app.add_file_reference @target_loginitem.product_reference
    app_file.settings = { "ATTRIBUTES" => ['RemoveHeadersOnCopy'] }
  end

  def die(msg)
   print $msg
   exit 1
  end
end

if ARGV.length < 3 || (ARGV[2] != "ios" && ARGV[2] != "macos")
  puts "Usage: <script> project_file version ios/macos"
  exit 1
end

platform = "macos"
platform = "ios" if ARGV[2] == "ios"

r = XCodeprojPatcher.new
r.run ARGV[0], ARGV[1], platform
exit 0
