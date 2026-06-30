/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#import "utils.h"

#include <arpa/inet.h>
#include <sys/sysctl.h>

// Aim to allocate roughly half the total core count as workers.
NSUInteger getWorkerCount() {
  constexpr const int min_workers = 2;

  int mib[] = { CTL_HW, HW_NCPU };
  int num_cores = min_workers * 2;
  size_t len = sizeof(num_cores);
  if (sysctl(mib, sizeof(mib)/sizeof(int), &num_cores, &len, nullptr, 0) < 0) {
    return min_workers;
  }
  
  int num_workers = num_cores / 2;
  return (num_workers < min_workers) ? min_workers : num_workers;
}

nw_endpoint_t convertEndpoint(NWEndpoint* old) {
  if (old == nil) {
    return nil;
  } else if ([old isKindOfClass:[NWBonjourServiceEndpoint class]]) {
    NWBonjourServiceEndpoint* service = (NWBonjourServiceEndpoint*)old;
    return nw_endpoint_create_bonjour_service([service.name UTF8String],
                                              [service.type UTF8String],
                                              [service.domain UTF8String]);
  } else if (![old isKindOfClass:[NWHostEndpoint class]]) {
    // Some endpoint type we don't support.
    return nil;
  }
  NWHostEndpoint* host = (NWHostEndpoint*)old;
  
  // If possible, try to convert it into an address endpoint.
  int port = host.port.intValue;
  if ([host.hostname containsString:@":"]) {
    struct sockaddr_in6 sin6;
    memset(&sin6, 0, sizeof(sin6));
    sin6.sin6_family = AF_INET6;
    sin6.sin6_len = sizeof(sin6);
    sin6.sin6_port = htons(port);
    if (inet_pton(AF_INET6, host.hostname.UTF8String, &sin6.sin6_addr.s6_addr)) {
      return nw_endpoint_create_address((struct sockaddr*)&sin6);
    }
  } else {
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_len = sizeof(sin);
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = inet_addr(host.hostname.UTF8String);
    if (sin.sin_addr.s_addr != INADDR_NONE) {
      return nw_endpoint_create_address((struct sockaddr*)&sin);
    }
  }

  return nw_endpoint_create_host(host.hostname.UTF8String, host.port.UTF8String);
}

NSError* vpnProviderError(NEProviderStopReason reason) {
  NSDictionary<NSString *,id>* info = nil;
  switch (reason) {
    case NEProviderStopReasonNone:
      info = @{NSLocalizedDescriptionKey: @"No specific reason"};
      break;

    case NEProviderStopReasonUserInitiated:
      info = @{NSLocalizedDescriptionKey: @"The user stopped the provider extension"};
      break;

    case NEProviderStopReasonProviderFailed:
      info = @{NSLocalizedDescriptionKey: @"The provider failed to function correctly"};
      break;

    case NEProviderStopReasonNoNetworkAvailable:
      info = @{NSLocalizedDescriptionKey: @"No network connectivity is currently available"};
      break;

    case NEProviderStopReasonUnrecoverableNetworkChange:
      info = @{NSLocalizedDescriptionKey: @"The device's network connectivity changed"};
      break;

    case NEProviderStopReasonProviderDisabled:
      info = @{NSLocalizedDescriptionKey: @"The provider was disabled"};
      break;

    case NEProviderStopReasonAuthenticationCanceled:
      info = @{NSLocalizedDescriptionKey: @"The authentication process was canceled"};
      break;

    case NEProviderStopReasonConfigurationFailed:
      info = @{NSLocalizedDescriptionKey: @"The configuration is invalid"};
      break;

    case NEProviderStopReasonIdleTimeout:
      info = @{NSLocalizedDescriptionKey: @"The session timed out"};
      break;

    case NEProviderStopReasonConfigurationDisabled:
      info = @{NSLocalizedDescriptionKey: @"The configuration was disabled"};
      break;

    case NEProviderStopReasonConfigurationRemoved:
      info = @{NSLocalizedDescriptionKey: @"The configuration was removed"};
      break;

    case NEProviderStopReasonSuperceded:
      info = @{NSLocalizedDescriptionKey: @"The configuration was superceded by a higher-priority configuration"};
      break;

    case NEProviderStopReasonUserLogout:
      info = @{NSLocalizedDescriptionKey: @"The user logged out"};
      break;

    case NEProviderStopReasonConnectionFailed:
      info = @{NSLocalizedDescriptionKey: @"The connection failed"};
      break;

    case NEProviderStopReasonSleep:
      info = @{NSLocalizedDescriptionKey: @"A stop reason indicating the configuration enabled disconnect on sleep and the device went to sleep"};
      break;

    case NEProviderStopReasonInternalError:
      info = @{NSLocalizedDescriptionKey: @"The provider encountered an internal error"};
      break;

    case NEProviderStopReasonAppUpdate:
    default:
      break;
  }

  return [NSError errorWithDomain:[[NSBundle mainBundle] bundleIdentifier]
                             code:(NSUInteger)reason
                         userInfo:info];
}

NSError* vpnPosixError(int code, NSString* desc) {
  NSString* msg;
  if (desc) {
    msg = [NSString stringWithFormat:@"%@: %s", desc, strerror(code)];
  } else {
    msg = [NSString stringWithFormat:@"%s", strerror(code)];
  }

  return [NSError errorWithDomain:NSPOSIXErrorDomain
                             code:code
                         userInfo:@{NSLocalizedDescriptionKey: msg}];
}
