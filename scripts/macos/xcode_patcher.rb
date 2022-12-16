# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

require 'xcodeproj'

class XCodeprojPatcher
  attr :project
  attr :qt_preprocess
  attr :target_main
  attr :target_extension

  def run(file, shortVersion, fullVersion, platform, configHash, adjust_sdk_token)
    open_project file
    open_target_main platform

    if platform == 'macos'
      setup_target_loginitem shortVersion, fullVersion, configHash
    end


    if platform == 'ios'
      setup_target_main configHash, adjust_sdk_token

      group = @project.main_group.new_group('Configuration')
      @configFile = group.new_file('xcode.xconfig')

      setup_target_extension shortVersion, fullVersion, configHash
      setup_target_gobridge
    end

    @project.save
  end

  def open_project(file)
    @project = Xcodeproj::Project.open(file)
    @qt_preprocess = @project.targets.find { |target| target.to_s == 'Qt Preprocess' }
    die 'Failed to open the project file: ' + file if @project.nil?
  end

  def open_target_main(platform)
    @target_main = @project.targets.find { |target| target.to_s == 'Mozilla VPN' } if platform == 'macos'
    @target_main = @project.targets.find { |target| target.to_s == 'MozillaVPN' } if platform == 'ios'
    return @target_main if not @target_main.nil?

    die 'Unable to open Mozilla VPN target'
  end

  def setup_target_main(configHash, adjust_sdk_token)
    @target_main.build_configurations.each do |config|
      config.base_configuration_reference = @configFile

      config.build_settings['FRAMEWORK_SEARCH_PATHS'] ||= [
        "$(inherited)",
        "$(PROJECT_DIR)/3rdparty",
        "$(PROJECT_DIR)/3rdparty/wireguard-apple/Sources/WireGuardKitC",
        "$(PROJECT_DIR)/3rdparty/wireguard-apple/Sources/WireGuardKitGo",
      ]

      config.build_settings['PRODUCT_NAME'] = 'Mozilla VPN'

      if adjust_sdk_token != ""
        config.build_settings['ADJUST_SDK_TOKEN'] = adjust_sdk_token
      end

      # Force xcode to not set QT_LIBRARY_SUFFIX to "_debug", which causes crash
      config.build_settings['QT_LIBRARY_SUFFIX'] = ""
      config.build_settings['GCC_PREPROCESSOR_DEFINITIONS'] ||= [
        'GROUP_ID=\"' + configHash['GROUP_ID_IOS'] + '\"',
        "VPN_NE_BUNDLEID=\\\"" + configHash['NETEXT_ID_IOS'] + "\\\"",
      ]
    end

    # WireGuard group
    group = @project.main_group.new_group('WireGuard')

    [
      '3rdparty/wireguard-apple/Sources/WireGuardKitGo/wireguard-go-version.h',
      '3rdparty/wireguard-apple/Sources/Shared/Keychain.swift',
      '3rdparty/wireguard-apple/Sources/WireGuardKit/IPAddressRange.swift',
      '3rdparty/wireguard-apple/Sources/WireGuardKit/InterfaceConfiguration.swift',
      '3rdparty/wireguard-apple/Sources/Shared/Model/NETunnelProviderProtocol+Extension.swift',
      '3rdparty/wireguard-apple/Sources/WireGuardKit/TunnelConfiguration.swift',
      '3rdparty/wireguard-apple/Sources/Shared/Model/TunnelConfiguration+WgQuickConfig.swift',
      '3rdparty/wireguard-apple/Sources/WireGuardKit/Endpoint.swift',
      '3rdparty/wireguard-apple/Sources/Shared/Model/String+ArrayConversion.swift',
      '3rdparty/wireguard-apple/Sources/WireGuardKit/PeerConfiguration.swift',
      '3rdparty/wireguard-apple/Sources/WireGuardKit/DNSServer.swift',
      '3rdparty/wireguard-apple/Sources/WireGuardApp/LocalizationHelper.swift',
      '3rdparty/wireguard-apple/Sources/Shared/FileManager+Extension.swift',
      '3rdparty/wireguard-apple/Sources/WireGuardKitC/x25519.c',
      '3rdparty/wireguard-apple/Sources/WireGuardKit/PrivateKey.swift',
    ].each { |filename|
      file = group.new_file(filename)
      @target_main.add_file_references([file])
    }

    # @target_main + swift integration
    group = @project.main_group.new_group('SwiftIntegration')

    [
      'src/apps/vpn/platforms/ios/ioscontroller.swift',
      'src/apps/vpn/platforms/ios/ioslogger.swift',
    ].each { |filename|
      file = group.new_file(filename)
      @target_main.add_file_references([file])
    }

    if adjust_sdk_token != ""
      frameworks_group = @project.groups.find { |group| group.display_name == 'Frameworks' }
      frameworks_build_phase = @target_main.build_phases.find { |build_phase| build_phase.to_s == 'FrameworksBuildPhase' }

      framework_ref = frameworks_group.new_file('AdServices.framework')
      build_file = frameworks_build_phase.add_file_reference(framework_ref)
      build_file.settings = { 'ATTRIBUTES' => ['Weak'] }

      framework_ref = frameworks_group.new_file('iAd.framework')
      frameworks_build_phase.add_file_reference(framework_ref)

      # Adjust SDK
      group = @project.main_group.new_group('AdjustSDK')

      [
        '3rdparty/adjust-ios-sdk/Adjust/ADJActivityHandler.h',
        '3rdparty/adjust-ios-sdk/Adjust/ADJActivityKind.h',
        '3rdparty/adjust-ios-sdk/Adjust/ADJActivityPackage.h',
        '3rdparty/adjust-ios-sdk/Adjust/ADJActivityState.h',
        '3rdparty/adjust-ios-sdk/Adjust/ADJAdjustFactory.h',
        '3rdparty/adjust-ios-sdk/Adjust/ADJAdRevenue.h',
        '3rdparty/adjust-ios-sdk/Adjust/ADJAttribution.h',
        '3rdparty/adjust-ios-sdk/Adjust/ADJAttributionHandler.h',
        '3rdparty/adjust-ios-sdk/Adjust/ADJBackoffStrategy.h',
        '3rdparty/adjust-ios-sdk/Adjust/ADJAdditions/NSData+ADJAdditions.h',
        '3rdparty/adjust-ios-sdk/Adjust/ADJAdditions/NSNumber+ADJAdditions.h',
        '3rdparty/adjust-ios-sdk/Adjust/ADJAdditions/NSString+ADJAdditions.h',
        '3rdparty/adjust-ios-sdk/Adjust/ADJConfig.h',
        '3rdparty/adjust-ios-sdk/Adjust/ADJEvent.h',
        '3rdparty/adjust-ios-sdk/Adjust/ADJEventFailure.h',
        '3rdparty/adjust-ios-sdk/Adjust/ADJEventSuccess.h',
        '3rdparty/adjust-ios-sdk/Adjust/ADJLinkResolution.h',
        '3rdparty/adjust-ios-sdk/Adjust/ADJLogger.h',
        '3rdparty/adjust-ios-sdk/Adjust/ADJPackageBuilder.h',
        '3rdparty/adjust-ios-sdk/Adjust/ADJPackageHandler.h',
        '3rdparty/adjust-ios-sdk/Adjust/ADJPackageParams.h',
        '3rdparty/adjust-ios-sdk/Adjust/ADJRequestHandler.h',
        '3rdparty/adjust-ios-sdk/Adjust/ADJResponseData.h',
        '3rdparty/adjust-ios-sdk/Adjust/ADJSdkClickHandler.h',
        '3rdparty/adjust-ios-sdk/Adjust/ADJSessionFailure.h',
        '3rdparty/adjust-ios-sdk/Adjust/ADJSessionParameters.h',
        '3rdparty/adjust-ios-sdk/Adjust/ADJSessionSuccess.h',
        '3rdparty/adjust-ios-sdk/Adjust/ADJSubscription.h',
        '3rdparty/adjust-ios-sdk/Adjust/ADJThirdPartySharing.h',
        '3rdparty/adjust-ios-sdk/Adjust/ADJTimerCycle.h',
        '3rdparty/adjust-ios-sdk/Adjust/ADJTimerOnce.h',
        '3rdparty/adjust-ios-sdk/Adjust/ADJUrlStrategy.h',
        '3rdparty/adjust-ios-sdk/Adjust/ADJUserDefaults.h',
        '3rdparty/adjust-ios-sdk/Adjust/Adjust.h',
        '3rdparty/adjust-ios-sdk/Adjust/ADJUtil.h',
        '3rdparty/adjust-ios-sdk/Adjust/ADJActivityHandler.m',
        '3rdparty/adjust-ios-sdk/Adjust/ADJActivityKind.m',
        '3rdparty/adjust-ios-sdk/Adjust/ADJActivityPackage.m',
        '3rdparty/adjust-ios-sdk/Adjust/ADJActivityState.m',
        '3rdparty/adjust-ios-sdk/Adjust/ADJAdjustFactory.m',
        '3rdparty/adjust-ios-sdk/Adjust/ADJAdRevenue.m',
        '3rdparty/adjust-ios-sdk/Adjust/ADJAttribution.m',
        '3rdparty/adjust-ios-sdk/Adjust/ADJAttributionHandler.m',
        '3rdparty/adjust-ios-sdk/Adjust/ADJBackoffStrategy.m',
        '3rdparty/adjust-ios-sdk/Adjust/ADJAdditions/NSData+ADJAdditions.m',
        '3rdparty/adjust-ios-sdk/Adjust/ADJAdditions/NSNumber+ADJAdditions.m',
        '3rdparty/adjust-ios-sdk/Adjust/ADJAdditions/NSString+ADJAdditions.m',
        '3rdparty/adjust-ios-sdk/Adjust/ADJConfig.m',
        '3rdparty/adjust-ios-sdk/Adjust/ADJEvent.m',
        '3rdparty/adjust-ios-sdk/Adjust/ADJEventFailure.m',
        '3rdparty/adjust-ios-sdk/Adjust/ADJEventSuccess.m',
        '3rdparty/adjust-ios-sdk/Adjust/ADJLinkResolution.m',
        '3rdparty/adjust-ios-sdk/Adjust/ADJLogger.m',
        '3rdparty/adjust-ios-sdk/Adjust/ADJPackageBuilder.m',
        '3rdparty/adjust-ios-sdk/Adjust/ADJPackageHandler.m',
        '3rdparty/adjust-ios-sdk/Adjust/ADJPackageParams.m',
        '3rdparty/adjust-ios-sdk/Adjust/ADJRequestHandler.m',
        '3rdparty/adjust-ios-sdk/Adjust/ADJResponseData.m',
        '3rdparty/adjust-ios-sdk/Adjust/ADJSdkClickHandler.m',
        '3rdparty/adjust-ios-sdk/Adjust/ADJSessionFailure.m',
        '3rdparty/adjust-ios-sdk/Adjust/ADJSessionParameters.m',
        '3rdparty/adjust-ios-sdk/Adjust/ADJSessionSuccess.m',
        '3rdparty/adjust-ios-sdk/Adjust/ADJSubscription.m',
        '3rdparty/adjust-ios-sdk/Adjust/ADJThirdPartySharing.m',
        '3rdparty/adjust-ios-sdk/Adjust/ADJTimerCycle.m',
        '3rdparty/adjust-ios-sdk/Adjust/ADJTimerOnce.m',
        '3rdparty/adjust-ios-sdk/Adjust/ADJUrlStrategy.m',
        '3rdparty/adjust-ios-sdk/Adjust/ADJUserDefaults.m',
        '3rdparty/adjust-ios-sdk/Adjust/Adjust.m',
        '3rdparty/adjust-ios-sdk/Adjust/ADJUtil.m',
      ].each { |filename|
        file = group.new_file(filename)
        file_reference = @target_main.add_file_references([file], '-fobjc-arc')
      }
    end
  end

  def setup_target_extension(shortVersion, fullVersion, configHash)
    @target_extension = @project.new_target(:app_extension, 'MozillaVPNNetworkExtension', :ios)

    @target_extension.build_configurations.each do |config|
      config.base_configuration_reference = @configFile

      config.build_settings['LD_RUNPATH_SEARCH_PATHS'] ||= '"$(inherited) @executable_path/../Frameworks"'
      config.build_settings['SWIFT_VERSION'] ||= '5.0'
      config.build_settings['CLANG_ENABLE_MODULES'] ||= 'YES'
      config.build_settings['SWIFT_OBJC_BRIDGING_HEADER'] ||= 'ios/networkextension/WireGuardNetworkExtension-Bridging-Header.h'
      config.build_settings['SWIFT_PRECOMPILE_BRIDGING_HEADER'] = 'NO'
      config.build_settings['APPLICATION_EXTENSION_API_ONLY'] = 'YES'

      # Versions and names
      config.build_settings['MARKETING_VERSION'] ||= shortVersion
      config.build_settings['CURRENT_PROJECT_VERSION'] ||= fullVersion
      config.build_settings['PRODUCT_BUNDLE_IDENTIFIER'] ||= configHash['NETEXT_ID_IOS']
      config.build_settings['PRODUCT_NAME'] = 'MozillaVPNNetworkExtension'

      # other configs
      config.build_settings['INFOPLIST_FILE'] ||= 'ios/networkextension/Info.plist'
      config.build_settings['CODE_SIGN_ENTITLEMENTS'] ||= 'ios/networkextension/MozillaVPNNetworkExtension.entitlements'
      config.build_settings['CODE_SIGN_IDENTITY'] = 'Apple Development'

      config.build_settings['ENABLE_BITCODE'] ||= 'NO'
      config.build_settings['SDKROOT'] = 'iphoneos'

      config.build_settings['FRAMEWORK_SEARCH_PATHS'] ||= [
        "$(PROJECT_DIR)/3rdparty/wireguard-apple/Sources/WireGuardKitC",
        "$(PROJECT_DIR)/3rdparty/wireguard-apple/Sources/WireGuardKitGo",
      ]
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

      groupId = configHash['GROUP_ID_IOS']

      config.build_settings['GCC_PREPROCESSOR_DEFINITIONS'] ||= [
        # This is needed to compile the iosglue without Qt.
        'NETWORK_EXTENSION=1',
        'GROUP_ID=\"' + groupId + '\"',
      ]

      if config.name == 'Release'
        config.build_settings['SWIFT_OPTIMIZATION_LEVEL'] ||= '-Onone'
      end

    end

    group = @project.main_group.new_group('WireGuardExtension')
    [
      '3rdparty/wireguard-apple/Sources/WireGuardKit/WireGuardAdapter.swift',
      '3rdparty/wireguard-apple/Sources/WireGuardKit/PacketTunnelSettingsGenerator.swift',
      '3rdparty/wireguard-apple/Sources/WireGuardKit/DNSResolver.swift',
      '3rdparty/wireguard-apple/Sources/WireGuardNetworkExtension/ErrorNotifier.swift',
      '3rdparty/wireguard-apple/Sources/Shared/Keychain.swift',
      '3rdparty/wireguard-apple/Sources/Shared/Model/TunnelConfiguration+WgQuickConfig.swift',
      '3rdparty/wireguard-apple/Sources/Shared/Model/NETunnelProviderProtocol+Extension.swift',
      '3rdparty/wireguard-apple/Sources/Shared/Model/String+ArrayConversion.swift',
      '3rdparty/wireguard-apple/Sources/WireGuardKit/TunnelConfiguration.swift',
      '3rdparty/wireguard-apple/Sources/WireGuardKit/IPAddressRange.swift',
      '3rdparty/wireguard-apple/Sources/WireGuardKit/Endpoint.swift',
      '3rdparty/wireguard-apple/Sources/WireGuardKit/DNSServer.swift',
      '3rdparty/wireguard-apple/Sources/WireGuardKit/InterfaceConfiguration.swift',
      '3rdparty/wireguard-apple/Sources/WireGuardKit/PeerConfiguration.swift',
      '3rdparty/wireguard-apple/Sources/Shared/FileManager+Extension.swift',
      '3rdparty/wireguard-apple/Sources/WireGuardKitC/x25519.c',
      '3rdparty/wireguard-apple/Sources/WireGuardKit/Array+ConcurrentMap.swift',
      '3rdparty/wireguard-apple/Sources/WireGuardKit/IPAddress+AddrInfo.swift',
      '3rdparty/wireguard-apple/Sources/WireGuardKit/PrivateKey.swift',
    ].each { |filename|
      file = group.new_file(filename)
      @target_extension.add_file_references([file])
    }
    # @target_extension + swift integration
    group = @project.main_group.new_group('SwiftIntegration')

    [
      'src/apps/vpn/platforms/ios/iostunnel.swift',
      'src/apps/vpn/platforms/ios/iosglue.mm',
      'src/apps/vpn/platforms/ios/ioslogger.swift',
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

    setup_target_dependency @target_main, @target_extension

    copy_appex = @target_main.new_copy_files_build_phase
    copy_appex.name = 'Copy Network-Extension plugin'
    copy_appex.symbol_dst_subfolder_spec = :plug_ins

    appex_file = copy_appex.add_file_reference @target_extension.product_reference
    appex_file.settings = { "ATTRIBUTES" => ['RemoveHeadersOnCopy'] }
  end

  def setup_target_gobridge
    target_gobridge = legacy_target = @project.new(Xcodeproj::Project::PBXLegacyTarget)

    target_gobridge.build_working_directory = '3rdparty/wireguard-apple/Sources/WireGuardKitGo'
    target_gobridge.build_tool_path = 'make'
    target_gobridge.pass_build_settings_in_environment = '1'
    target_gobridge.build_arguments_string = '$(ACTION)'
    target_gobridge.name = 'WireGuardGoBridge'
    target_gobridge.product_name = 'WireGuardGoBridge'

    @project.targets << target_gobridge
    @target_extension.add_dependency target_gobridge
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

    setup_target_dependency @target_loginitem, @qt_preprocess
    setup_target_dependency @target_main, @target_loginitem

    copy_app = @target_main.new_copy_files_build_phase
    copy_app.name = 'Copy LoginItem'
    copy_app.symbol_dst_subfolder_spec = :wrapper
    copy_app.dst_path = 'Contents/Library/LoginItems'

    app_file = copy_app.add_file_reference @target_loginitem.product_reference
    app_file.settings = { "ATTRIBUTES" => ['RemoveHeadersOnCopy'] }
  end

  # This works around target.add_dependency failing by creating a PBXContainerItemProxy.
  def setup_target_dependency(target, depends)
    container_proxy = @project.new(Xcodeproj::Project::PBXContainerItemProxy)
    container_proxy.container_portal = @project.root_object.uuid
    container_proxy.proxy_type = Xcodeproj::Constants::PROXY_TYPES[:native_target]
    container_proxy.remote_global_id_string = depends.uuid
    container_proxy.remote_info = depends.name

    target_dependency = @project.new(Xcodeproj::Project::PBXTargetDependency)
    target_dependency.name = depends.name
    target_dependency.target = target
    target_dependency.target_proxy = container_proxy

    target.dependencies << target_dependency
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
adjust_sdk_token = ARGV[4]

r = XCodeprojPatcher.new
r.run ARGV[0], ARGV[1], ARGV[2], platform, config, adjust_sdk_token
exit 0
