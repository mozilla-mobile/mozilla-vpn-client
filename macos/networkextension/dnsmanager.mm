/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#import <CoreFoundation/CoreFoundation.h>
#import <Foundation/Foundation.h>
#import <SystemConfiguration/SystemConfiguration.h>

#include <getopt.h>
#include <sys/event.h>
#include <sys/syslimits.h>
#include <unistd.h>
#include "utils.h"

static const char* scErrorMessage() {
  return SCErrorString(SCError());
}

static void usage(int argc, char** argv) {
    printf("Usage: %s [options] SERVER [SERVER ...]\n\n", argv[0]);

    printf("Update macOS DNS configuration\n\n");
}

static int version() {
  CFBundleRef bundle = CFBundleGetMainBundle();
  CFStringRef name = CFBundleGetIdentifier(bundle);
  CFStringRef key = CFSTR("CFBundleVersion");
  CFTypeRef version = CFBundleGetValueForInfoDictionaryKey(bundle, key);

  CFStringRef result = CFStringCreateWithFormat(
      kCFAllocatorSystemDefault, nullptr, CFSTR("%@ version %@"), name, version);
  if (!result) {
    return 1;
  }

  char buffer[256];
  if (!CFStringGetCString(result, buffer, sizeof(buffer), kCFStringEncodingUTF8)) {
    return 1;
  }

  puts(buffer);
  return 0;
}

static void restoreSnapshot(SCDynamicStoreRef store, NSDictionary* snapshot) {
  [snapshot enumerateKeysAndObjectsUsingBlock:^(NSString* uuid, NSDictionary* config, BOOL* stop){
    CFStringRef dnsPath = CFStringCreateWithFormat(
        kCFAllocatorSystemDefault, nullptr,
        CFSTR("Setup:/Network/Service/%s/DNS"), uuid.UTF8String);

    // Restore the snapshotted DNS configuration for this service.
    if ([config isKindOfClass:[NSDictionary class]]) {
      fprintf(stderr, "Restoring DNS for %s\n", uuid.UTF8String);
      SCDynamicStoreSetValue(store, dnsPath, (__bridge CFPropertyListRef)config);
    } else {
      fprintf(stderr, "Removing DNS for %s\n", uuid.UTF8String);
      SCDynamicStoreRemoveValue(store, dnsPath);
    }
    CFRelease(dnsPath);
  }];
}

static NSArray<NSString*>* parseServers(int argc, char* argv[]) {
  // The DNS server addresses should remain as the positional arguments.
  NSMutableArray* dnsAddrList = [NSMutableArray arrayWithCapacity:argc];
  for (int i = 0; i < argc; i++) {
    NSString* addr = [[NSString alloc] initWithBytes:argv[i]
                                              length:strlen(argv[i])
                                            encoding:NSUTF8StringEncoding];
    [dnsAddrList addObject:addr];
  }
  return [NSArray arrayWithArray:dnsAddrList];
}

static NSDictionary* parseConfig(int argc, char* argv[]) {
  NSMutableDictionary* dict = [NSMutableDictionary new];
  [dict setObject:@"lan" forKey:(__bridge NSString*)kSCPropNetDNSDomainName];

  // Parse options
  const char* shortopts = "h";
  const struct option longopts[] = {
    {"domain", required_argument, 0, 'd'},
    {"port",   required_argument, 0, 'p'},
    {NULL, 0, 0, 0},
  };

  optind = 0;
  while (true) {
    int c = getopt_long(argc, argv, shortopts, longopts, NULL);
    if (c < 0) {
      break;
    }

    NSString* value = nil;
    if (optarg) {
      value = (__bridge_transfer NSString*)CFStringCreateWithCString(NULL, optarg, kCFStringEncodingUTF8);
    }
  
    switch (c) {
      case 'd':
        [dict setObject:value forKey:(__bridge NSString*)kSCPropNetDNSDomainName];
        break;

      case 'p':
        [dict setObject:[NSNumber numberWithInteger:value.integerValue]
                 forKey:(__bridge NSString*)kSCPropNetDNSServerPort];
        break;

      default:
        continue;
    }
  }

  // The DNS server addresses should remain as the positional arguments.
  NSMutableArray* dnsAddrList = [NSMutableArray arrayWithCapacity:argc-optind];
  for (int i = optind; i < argc; i++) {
    NSString* addr = [[NSString alloc] initWithBytes:argv[i]
                                              length:strlen(argv[i])
                                            encoding:NSUTF8StringEncoding];
    [dnsAddrList addObject:addr];
  }
  if (dnsAddrList.count == 0) {
    fprintf(stderr, "Invalid configuration: no DNS servers were provided\n");
    return nil;
  }

  [dict setObject:[NSArray arrayWithArray:dnsAddrList]
           forKey:(__bridge NSString*)kSCPropNetDNSServerAddresses];
  return [NSDictionary dictionaryWithDictionary:dict];
}

// Get the list of current network services and take a snapshot.
static NSDictionary* takeSnapshot(SCDynamicStoreRef store) {
  CFStringRef filter = CFSTR("Setup:/Network/Service/[0-9A-Fa-f-]+");
  CFArrayRef keys = SCDynamicStoreCopyKeyList(store, filter);
  if (!keys) {
    fprintf(stderr, "Unble to enumerate network services\n");
    return [NSDictionary new];
  }

  NSMutableDictionary* snapshot = [NSMutableDictionary new];
  for (CFIndex i = 0; i < CFArrayGetCount(keys); i++) {
    NSString* path = (__bridge NSString*)CFArrayGetValueAtIndex(keys, i);
    NSString* uuid = [[path componentsSeparatedByString:@"/"] lastObject];

    fprintf(stderr, "Found network service: %s\n", uuid.UTF8String);
    CFStringRef dnsPath = CFStringCreateWithFormat(
        kCFAllocatorSystemDefault, nullptr,
        CFSTR("Setup:/Network/Service/%s/DNS"), uuid.UTF8String);

    // Take a snapshot of the existing configuration.
    CFPropertyListRef prevConfig = SCDynamicStoreCopyValue(store, dnsPath);
    if (prevConfig == nil) {
      [snapshot setObject:[NSNull null] forKey:uuid];
    } else if (CFGetTypeID(prevConfig) == CFDictionaryGetTypeID()) {
      [snapshot setObject:(__bridge_transfer NSDictionary*)prevConfig forKey:uuid];
    } else {
      CFRelease(prevConfig);
    }

    CFRelease(dnsPath);
  }

  return [NSDictionary dictionaryWithDictionary: snapshot];
}

static void applyUpdate(NSDictionary* config, NSArray<NSString*>* services, SCDynamicStoreRef store) {
  if (config == nil) {
    fprintf(stderr, "Invalid configuration\n");
    return;
  }

  for (NSString* uuid : services) {
    fprintf(stderr, "Updating network service: %s\n", uuid.UTF8String);
    CFStringRef dnsPath = CFStringCreateWithFormat(
        kCFAllocatorSystemDefault, nullptr,
        CFSTR("Setup:/Network/Service/%s/DNS"), uuid.UTF8String);
    if (!dnsPath) {
      continue;
    }

    if (!SCDynamicStoreSetValue(store, dnsPath, (__bridge CFPropertyListRef)config)) {
      fprintf(stderr, "Failed to update configuration for: %s\n", uuid.UTF8String);;
      fprintf(stderr, "Error: %s\n", scErrorMessage());
    }

    CFRelease(dnsPath);
  }
}

// Block the process and wait for a termination signal.
static int waitForTermination(int kq) {
  struct kevent ev;
  int ret = kevent(kq, nullptr, 0, &ev, 1, nullptr);
  if ((ret < 0) && (errno == EINTR)) {
    // Received a signal.
    return SIGINT;
  } else if ((ret <= 0) || (ev.flags & EV_ERROR)) {
    // Something went horribly wrong.
    return SIGKILL;
  } else if (ev.filter == EVFILT_SIGNAL) {
    // Received a signal.
    return ev.ident;
  } else if (ev.filter == EVFILT_READ) {
    // Data is ready on stdin.
    return SIGHUP;
  } else if (ev.filter == EVFILT_PROC) {
    // Received NOTE_EXIT; our parent has terminated.
    return SIGCHLD;
  }

  // Something went horribly wrong.
  return SIGKILL;
}

int dnsManagerMain(int argc, char* argv[]) {
  // Scan for special arguments that only apply on the command line.
  for (int i = 1; i < argc; i++) {
    char* p = argv[i];
    if ((strcmp(p, "-h") == 0) || (strcmp(p, "--help") == 0)) {
      usage(argc, argv);
      return 0;
    }
    if ((strcmp(p, "-v") == 0) || (strcmp(p, "--version") == 0)) {
      return version();
    }
  }

  // Prepare the updated DNS configuration.
  NSDictionary* dnsConfig = parseConfig(argc, argv);
  if (!dnsConfig) {
    usage(argc, argv);
    return 0;
  }

  // Reconfigure stdin to receive updates from the parent process.
  int flags = fcntl(STDIN_FILENO, F_GETFL, 0) | O_NONBLOCK;
  fcntl(STDIN_FILENO, F_SETFL, flags);

  // Open the system configuration store.
  CFStringRef name = CFSTR("mozillavpn");
  SCDynamicStoreRef store =
      SCDynamicStoreCreate(kCFAllocatorSystemDefault, name, nullptr, nullptr);
  if (store == nil) {
    fprintf(stderr, "Failed to open system configuration: %s\n", scErrorMessage());
    return 1;
  }

  // Take a snapshot of the DNS configuration.
  NSDictionary* snapshot = takeSnapshot(store);
  applyUpdate(dnsConfig, snapshot.allKeys, store);

  // Watch for NOTE_EXIT on our parent process.
  // https://developer.apple.com/library/archive/technotes/tn2050/_index.html
  struct kevent changes[6];
  EV_SET(&changes[0], getppid(), EVFILT_PROC, EV_ADD | EV_RECEIPT, NOTE_EXIT, 0, NULL);
  EV_SET(&changes[1], STDIN_FILENO, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
  EV_SET(&changes[2], SIGINT, EVFILT_SIGNAL, EV_ADD | EV_ENABLE, 0, 0, NULL);
  EV_SET(&changes[3], SIGTERM, EVFILT_SIGNAL, EV_ADD | EV_ENABLE, 0, 0, NULL);
  EV_SET(&changes[4], SIGHUP, EVFILT_SIGNAL, EV_ADD | EV_ENABLE, 0, 0, NULL);
  EV_SET(&changes[5], SIGPIPE, EVFILT_SIGNAL, EV_ADD | EV_ENABLE, 0, 0, NULL);

  signal(SIGINT, SIG_IGN);
  signal(SIGTERM, SIG_IGN);
  signal(SIGHUP, SIG_IGN);
  signal(SIGPIPE, SIG_IGN);

  // Wait for termination
  int kq = kqueue();
  kevent(kq, changes, sizeof(changes)/sizeof(struct kevent), nullptr, 0, nullptr);
  while (waitForTermination(kq) == SIGHUP) {
    char buffer[ARG_MAX];
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
      continue;
    }
    // Ensure termination.
    buffer[ARG_MAX-1] = '\0';

    // Permute it into an argc/argv array by splitting on whitespace.
    const char* delim = " \t\n";
    char* command[512];
    int nargs = 0;
    char* end = NULL;
    char* tok = strtok_r(buffer, delim, &end);
    while (tok && (nargs < 512)) {
      command[nargs++] = tok;
      tok = strtok_r(NULL, delim, &end);
    }

    // Handle commands received via stdin.
    if (strcmp(command[0], "update") == 0) {
      dnsConfig = parseConfig(nargs, command);
      if (!dnsConfig) {
        continue;
      }
      applyUpdate(dnsConfig, snapshot.allKeys, store);
    }
  }
  close(kq);

  // Restore the DNS configuration on exit.
  restoreSnapshot(store, snapshot);
  CFRelease(store);
  return 0;
}
