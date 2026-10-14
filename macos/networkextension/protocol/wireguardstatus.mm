/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "wireguardstatus.h"

@implementation WireguardStatus
+ (BOOL)supportsSecureCoding {
  return YES;
}

- (void)encodeWithCoder:(NSCoder *)coder {
  [coder encodeObject:self.lastHandshake forKey:@"lastHandshake"];
  [coder encodeObject:self.ipv4address forKey:@"ipv4address"];
  [coder encodeObject:self.ipv6address forKey:@"ipv6address"];
  [coder encodeObject:self.ipv4gateway forKey:@"ipv4gateway"];
  [coder encodeObject:self.ipv6gateway forKey:@"ipv6gateway"];
  [coder encodeInt64:self.rxBytes forKey:@"rxBytes"];
  [coder encodeInt64:self.txBytes forKey:@"txBytes"];
  [coder encodeFloat:self.estimatedLoss forKey:@"estimatedLoss"];
  [coder encodeInt:self.estimatedRtt forKey:@"estimatedRtt"];
}

- (id)initWithCoder:(NSCoder*)coder {
  self = [super init];
  self.lastHandshake = [coder decodeObjectOfClass:[NSDate class] forKey:@"lastHandshake"];
  self.ipv4address = [coder decodeObjectOfClass:[NSString class] forKey:@"ipv4address"];
  self.ipv6address = [coder decodeObjectOfClass:[NSString class] forKey:@"ipv6address"];
  self.ipv4gateway = [coder decodeObjectOfClass:[NSString class] forKey:@"ipv4gateway"];
  self.ipv6gateway = [coder decodeObjectOfClass:[NSString class] forKey:@"ipv6gateway"];
  self.rxBytes = [coder decodeInt64ForKey:@"rxBytes"];
  self.txBytes = [coder decodeInt64ForKey:@"txBytes"];
  self.estimatedLoss = [coder decodeFloatForKey:@"estimatedLoss"];
  self.estimatedRtt = [coder decodeIntForKey:@"estimatedRtt"];
  return self;
}

@end
