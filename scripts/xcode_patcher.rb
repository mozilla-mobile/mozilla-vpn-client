# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

require 'xcodeproj'

class XCodeprojPatcher
  attr :project
  attr :target_main
  attr :target_extension

  def run(file, shortVersion, fullVersion, platform, configHash)
    open_project file
    open_target_main

    group = @project.main_group.new_group('Configuration')
    @configFile = group.new_file('xcode.xconfig')

    setup_target_main shortVersion, fullVersion, platform, configHash
    setup_target_extension shortVersion, fullVersion, platform, configHash

    setup_target_loginitem shortVersion, fullVersion, configHash if platform == "macos"

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

    die 'Unable to open MozillaVPN target'
  end

  def setup_target_main(shortVersion, fullVersion, platform, configHash)
    @target_main.build_configurations.each do |config|
      config.base_configuration_reference = @configFile

      config.build_settings['LD_RUNPATH_SEARCH_PATHS'] ||= '"$(inherited) @executable_path/../Frameworks"'
      config.build_settings['SWIFT_VERSION'] ||= '5.0'
      config.build_settings['CLANG_ENABLE_MODULES'] ||= 'YES'
      config.build_settings['SWIFT_OBJC_BRIDGING_HEADER'] ||= 'macos/app/WireGuard-Bridging-Header.h'

      # Versions and names
      config.build_settings['MARKETING_VERSION'] ||= shortVersion
      config.build_settings['CURRENT_PROJECT_VERSION'] ||= fullVersion
      config.build_settings['PRODUCT_BUNDLE_IDENTIFIER'] = configHash['APP_ID_MACOS'] if platform == 'macos'
      config.build_settings['PRODUCT_BUNDLE_IDENTIFIER'] = configHash['APP_ID_IOS'] if platform == 'ios'
      config.build_settings['PRODUCT_NAME'] = 'Mozilla VPN'

      # other config
      config.build_settings['INFOPLIST_FILE'] ||= platform + '/app/Info.plist'
      config.build_settings['CODE_SIGN_ENTITLEMENTS'] ||= platform +'/app/MozillaVPN.entitlements'
      config.build_settings['CODE_SIGN_IDENTITY'] ||= 'Apple Development'
      config.build_settings['ENABLE_BITCODE'] ||= 'NO' if platform == 'ios'
      config.build_settings['SDKROOT'] = 'iphoneos' if platform == 'ios'

      groupId = "";
      if (platform == 'macos')
        groupId = configHash['DEVELOPMENT_TEAM'] + "." + configHash['GROUP_ID_MACOS']
      else
        groupId = configHash['GROUP_ID_IOS']
      end

      config.build_settings['GCC_PREPROCESSOR_DEFINITIONS'] ||= [
        'GROUP_ID=\"' + groupId + '\"',
      ]

      if config.name == 'Release'
        config.build_settings['SWIFT_OPTIMIZATION_LEVEL'] ||= '-Onone'
      end
    end

    # WireGuard group
    group = @project.main_group.new_group('WireGuard')

    [
      'macos/gobridge/wireguard-go-version.h',
      '3rdparty/wireguard-apple/WireGuard/Shared/Keychain.swift',
      '3rdparty/wireguard-apple/WireGuard/Shared/Model/Data+KeyEncoding.swift',
      '3rdparty/wireguard-apple/WireGuard/Shared/Model/IPAddressRange.swift',
      '3rdparty/wireguard-apple/WireGuard/Shared/Model/InterfaceConfiguration.swift',
      '3rdparty/wireguard-apple/WireGuard/Shared/Model/NETunnelProviderProtocol+Extension.swift',
      '3rdparty/wireguard-apple/WireGuard/Shared/Model/TunnelConfiguration.swift',
      '3rdparty/wireguard-apple/WireGuard/Shared/Model/TunnelConfiguration+WgQuickConfig.swift',
      '3rdparty/wireguard-apple/WireGuard/Shared/Model/Endpoint.swift',
      '3rdparty/wireguard-apple/WireGuard/Shared/Model/String+ArrayConversion.swift',
      '3rdparty/wireguard-apple/WireGuard/Shared/Model/PeerConfiguration.swift',
      '3rdparty/wireguard-apple/WireGuard/Shared/Model/DNSServer.swift',
      '3rdparty/wireguard-apple/WireGuard/WireGuard/LocalizationHelper.swift',
      '3rdparty/wireguard-apple/WireGuard/Shared/FileManager+Extension.swift',
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

  def setup_target_extension(shortVersion, fullVersion, platform, configHash)
    @target_extension = @project.new_target(:app_extension, 'WireGuardNetworkExtension', :osx)

    @target_extension.build_configurations.each do |config|
      config.base_configuration_reference = @configFile

      config.build_settings['LD_RUNPATH_SEARCH_PATHS'] ||= '"$(inherited) @executable_path/../Frameworks"'
      config.build_settings['SWIFT_VERSION'] ||= '5.0'
      config.build_settings['CLANG_ENABLE_MODULES'] ||= 'YES'
      config.build_settings['SWIFT_OBJC_BRIDGING_HEADER'] ||= 'macos/networkextension/WireGuardNetworkExtension-Bridging-Header.h'

      # Versions and names
      config.build_settings['MARKETING_VERSION'] ||= shortVersion
      config.build_settings['CURRENT_PROJECT_VERSION'] ||= fullVersion
      config.build_settings['PRODUCT_BUNDLE_IDENTIFIER'] ||= configHash['NETEXT_ID_MACOS'] if platform == 'macos'
      config.build_settings['PRODUCT_BUNDLE_IDENTIFIER'] ||= configHash['NETEXT_ID_IOS'] if platform == 'ios'
      config.build_settings['PRODUCT_NAME'] = 'WireGuardNetworkExtension'

      # other configs
      config.build_settings['INFOPLIST_FILE'] ||= 'macos/networkextension/Info.plist'
      config.build_settings['CODE_SIGN_ENTITLEMENTS'] ||= platform + '/networkextension/MozillaVPNNetworkExtension.entitlements'
      config.build_settings['CODE_SIGN_IDENTITY'] = 'Apple Development'

      if platform == 'ios'
        config.build_settings['ENABLE_BITCODE'] ||= 'NO'
        config.build_settings['SDKROOT'] = 'iphoneos'

        config.build_settings['OTHER_LDFLAGS'] ||= [
          "-stdlib=libc++",
          "-Wl,-rpath,@executable_path/Frameworks",
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

      groupId = "";
      if (platform == 'macos')
        groupId = configHash['DEVELOPMENT_TEAM'] + "." + configHash['GROUP_ID_MACOS']
      else
        groupId = configHash['GROUP_ID_IOS']
      end

      config.build_settings['GCC_PREPROCESSOR_DEFINITIONS'] ||= [
        # This is needed to compile the macosglue without Qt.
        'MACOS_EXTENSION=1',
        'GROUP_ID=\"' + groupId + '\"',
      ]

      if config.name == 'Release'
        config.build_settings['SWIFT_OPTIMIZATION_LEVEL'] ||= '-Onone'
      end

    end

    group = @project.main_group.new_group('WireGuardExtension')
    [
      '3rdparty/wireguard-apple/WireGuard/WireGuardNetworkExtension/PacketTunnelProvider.swift',
      '3rdparty/wireguard-apple/WireGuard/WireGuardNetworkExtension/PacketTunnelSettingsGenerator.swift',
      '3rdparty/wireguard-apple/WireGuard/WireGuardNetworkExtension/DNSResolver.swift',
      '3rdparty/wireguard-apple/WireGuard/WireGuardNetworkExtension/ErrorNotifier.swift',
      '3rdparty/wireguard-apple/WireGuard/Shared/Keychain.swift',
      '3rdparty/wireguard-apple/WireGuard/Shared/Model/TunnelConfiguration+WgQuickConfig.swift',
      '3rdparty/wireguard-apple/WireGuard/Shared/Model/NETunnelProviderProtocol+Extension.swift',
      '3rdparty/wireguard-apple/WireGuard/Shared/Model/String+ArrayConversion.swift',
      '3rdparty/wireguard-apple/WireGuard/Shared/Model/TunnelConfiguration.swift',
      '3rdparty/wireguard-apple/WireGuard/Shared/Model/Data+KeyEncoding.swift',
      '3rdparty/wireguard-apple/WireGuard/Shared/Model/IPAddressRange.swift',
      '3rdparty/wireguard-apple/WireGuard/Shared/Model/Endpoint.swift',
      '3rdparty/wireguard-apple/WireGuard/Shared/Model/DNSServer.swift',
      '3rdparty/wireguard-apple/WireGuard/Shared/Model/InterfaceConfiguration.swift',
      '3rdparty/wireguard-apple/WireGuard/Shared/Model/PeerConfiguration.swift',
      '3rdparty/wireguard-apple/WireGuard/Shared/FileManager+Extension.swift',
    ].each { |filename|
      file = group.new_file(filename)
      @target_extension.add_file_references([file])
    }

    # @target_extension + swift integration
    group = @project.main_group.new_group('SwiftIntegration')

    [
      'src/platforms/macos/macosglue.mm',
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

    target_go.build_working_directory = '3rdparty/wireguard-apple/wireguard-go-bridge'
    target_go.build_tool_path = 'make'
    target_go.pass_build_settings_in_environment = '1'
    target_go.build_arguments_string = '$(ACTION)'
    target_go.name = 'WireGuardGoBridge'
    target_go.product_name = 'WireGuardGoBridge'

    @project.targets << target_go

    @target_extension.add_dependency target_go
  end

  def setup_target_loginitem(shortVersion, fullVersion, configHash)
    @target_loginitem = @project.new_target(:application, 'MozillaVPNLoginItem', :osx)

    @target_loginitem.build_configurations.each do |config|
      config.base_configuration_reference = @configFile

      config.build_settings['LD_RUNPATH_SEARCH_PATHS'] ||= '"$(inherited) @executable_path/../Frameworks"'

      # Versions and names
      config.build_settings['MARKETING_VERSION'] ||= shortVersion
      config.build_settings['CURRENT_PROJECT_VERSION'] ||= fullVersion
      config.build_settings['PRODUCT_BUNDLE_IDENTIFIER'] ||= configHash['LOGIN_ID_MACOS']
      config.build_settings['PRODUCT_NAME'] = 'MozillaVPNLoginItem'

      # other configs
      config.build_settings['INFOPLIST_FILE'] ||= 'macos/loginitem/Info.plist'
      config.build_settings['CODE_SIGN_ENTITLEMENTS'] ||= 'macos/loginitem/MozillaVPNLoginItem.entitlements'
      config.build_settings['CODE_SIGN_IDENTITY'] = 'Apple Development'
      config.build_settings['SKIP_INSTALL'] = 'YES'

      config.build_settings['GCC_PREPROCESSOR_DEFINITIONS'] ||= [
        'APP_ID=\"' + configHash['APP_ID_MACOS'] + '\"',
      ]

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

if ARGV.length < 4 || (ARGV[3] != "ios" && ARGV[3] != "macos")
  puts "Usage: <script> project_file shortVersion fullVersion ios/macos"
  exit 1
end

if !File.exist?("xcode.xconfig")
  puts "xcode.xconfig file is required! See the template file."
  exit 1
end

config = Hash.new
configFile = File.read("xcode.xconfig").split("\n")
configFile.each { |line|
  next if line[0] == "#"

  if line.include? "="
    keys = line.split("=")
    config[keys[0].strip] = keys[1].strip
  end
}

platform = "macos"
platform = "ios" if ARGV[3] == "ios"

r = XCodeprojPatcher.new
r.run ARGV[0], ARGV[1], ARGV[2], platform, config
exit 0
