/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "iosadjusthelper.h"
#include "logger.h"
#include "constants.h"

#import "Adjust.h"
#import "ADJAdjustFactory.h"

namespace {

Logger logger("IOSAdjustHelper");

}  // namespace

void IOSAdjustHelper::initialize(quint16 proxyPort) {
  NSString *adjustToken = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"ADJUST_SDK_TOKEN"];

  if(adjustToken.length) {
    logger.debug() << "Successfully found Adjust token in info.plist.";
    NSString *environment = Constants::inProduction() ? ADJEnvironmentProduction : ADJEnvironmentSandbox;
    ADJConfig *adjustConfig = [ADJConfig configWithAppToken:adjustToken
                                                environment:environment];
    [adjustConfig setLogLevel:ADJLogLevelDebug];
    NSString* proxyAddress = [NSString stringWithFormat:@"http://127.0.0.1:%d", proxyPort];
    [ADJAdjustFactory setBaseUrl:proxyAddress];
    [ADJAdjustFactory setGdprUrl:proxyAddress];
    [Adjust appDidLaunch:adjustConfig];
  } else {
    logger.debug() << "Did not find Adjust token in info.plist.";
  }
}

void IOSAdjustHelper::trackEvent(const QString& eventToken) {
  NSString *adjustToken = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"ADJUST_SDK_TOKEN"];

  if(adjustToken.length) {
    ADJEvent *event = [ADJEvent eventWithEventToken:eventToken.toNSString()];
    [Adjust trackEvent:event];
  }
}

void IOSAdjustHelper::forget() {
  NSString* adjustToken = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"ADJUST_SDK_TOKEN"];

  if (adjustToken.length) {
    [Adjust gdprForgetMe];
  }
}
