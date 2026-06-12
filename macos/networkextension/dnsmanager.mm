/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#import <CoreFoundation/CoreFoundation.h>
#import <Foundation/Foundation.h>
#import <SystemConfiguration/SystemConfiguration.h>

#include <sys/event.h>
#include <unistd.h>


static const char* scErrorMessage() {
  return SCErrorString(SCError());
}

static void usage(int argc, char** argv) {
    printf("Usage: %s [options] SERVER [SERVER ...]\n\n", argv[0]);

    printf("Update macOS DNS configuration\n\n");
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

static NSArray<NSString*>* enumerateNetServices(SCDynamicStoreRef store) {
  CFStringRef filter = CFSTR("Setup:/Network/Service/[0-9A-Fa-f-]+");
  CFArrayRef keys = SCDynamicStoreCopyKeyList(store, filter);
  
  // Parse the UUID out of the strings
  NSMutableArray* result = [NSMutableArray arrayWithCapacity:CFArrayGetCount(keys)];
  for (CFIndex i = 0; i < CFArrayGetCount(keys); i++) {
    NSString* path = (__bridge NSString*)CFArrayGetValueAtIndex(keys, i);
    NSString* uuid = [[path componentsSeparatedByString:@"/"] lastObject];
    [result addObject:uuid];
  }

  return result;
}

// Block the process and wait for a termination signal.
static int waitForTermination(void) {
  int kq = kqueue();

  // Watch for NOTE_EXIT on our parent process.
  // https://developer.apple.com/library/archive/technotes/tn2050/_index.html
  struct kevent changes;
  EV_SET(&changes, getppid(), EVFILT_PROC, EV_ADD | EV_RECEIPT, NOTE_EXIT, 0,
         nullptr);
  kevent(kq, &changes, 1, &changes, 1, nullptr);

  // Prepare to handle termination signals.
  static int sig = 0;
  signal(SIGINT, [](int s) { sig = s; });
  signal(SIGTERM, [](int s) { sig = s; });
  signal(SIGHUP, [](int s) { sig = s; });
  signal(SIGPIPE, [](int s) { sig = s; });

  struct kevent ev;
  int ret = kevent(kq, nullptr, 0, &ev, 1, nullptr);
  close(kq);
  if ((ret < 0) && (errno == EINTR)) {
    // Received a signal.
    return sig;
  } else if ((ret <= 0) || (ev.flags & EV_ERROR)) {
    // Something went horribly wrong.
    return SIGKILL;
  } else {
    // Received NOTE_EXIT; our parent has terminated.
    return SIGCHLD;
  }
}

int main(int argc, char** argv) {
  const char* shortopts = "h";

  // Parse options
  while (true) {
    int c = getopt(argc, argv, shortopts);
    if (c < 0) {
      break;
    }
    switch (c) {
      case 'h':
        usage(argc, argv);
        return 0;
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

  // Prepare the updated DNS configuration.
  NSDictionary* dnsConfig = @{
    (__bridge NSString*)kSCPropNetDNSServerAddresses: dnsAddrList,
    (__bridge NSString*)kSCPropNetDNSDomainName: @"lan",
  };

  // Open the system configuration store.
  CFStringRef name = CFSTR("mozillavpn");
  SCDynamicStoreRef store =
      SCDynamicStoreCreate(kCFAllocatorSystemDefault, name, nullptr, nullptr);
  if (store == nil) {
    fprintf(stderr, "Failed to open system configuration: %s\n", scErrorMessage());
    return 1;
  }

  // Get the list of current network services and take a snapshot.
  NSMutableDictionary* snapshot = [NSMutableDictionary new];
  for (NSString* uuid : enumerateNetServices(store)) {
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

    // Replace the configuration
    if (!SCDynamicStoreSetValue(store, dnsPath, (__bridge CFPropertyListRef)dnsConfig)) {
      fprintf(stderr, "Failed to update configuration for: %s\n", uuid.UTF8String);;
      fprintf(stderr, "Error: %s\n", scErrorMessage());
      [snapshot removeObjectForKey:uuid];
    }

    CFRelease(dnsPath);
  }

  // Wait for a termination reason and restore the configuration on exit.
  waitForTermination();
  restoreSnapshot(store, snapshot);
  CFRelease(store);
  return 0;
}
