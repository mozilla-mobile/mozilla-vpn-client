package com.adjust.sdk;

public enum ActivityKind {
  UNKNOWN,
  SESSION,
  EVENT,
  CLICK,
  ATTRIBUTION,
  REVENUE,
  REATTRIBUTION,
  INFO,
  GDPR,
  AD_REVENUE,
  DISABLE_THIRD_PARTY_SHARING,
  SUBSCRIPTION,
  THIRD_PARTY_SHARING,
  MEASUREMENT_CONSENT;

  public static ActivityKind fromString(String string) {
    if ("session".equals(string)) {
      return SESSION;
    } else if ("event".equals(string)) {
      return EVENT;
    } else if ("click".equals(string)) {
      return CLICK;
    } else if ("attribution".equals(string)) {
      return ATTRIBUTION;
    } else if ("info".equals(string)) {
      return INFO;
    } else if ("gdpr".equals(string)) {
      return GDPR;
    } else if ("disable_third_party_sharing".equals(string)) {
      return DISABLE_THIRD_PARTY_SHARING;
    } else if ("ad_revenue".equals(string)) {
      return AD_REVENUE;
    } else if ("subscription".equals(string)) {
      return SUBSCRIPTION;
    } else if ("third_party_sharing".equals(string)) {
      return THIRD_PARTY_SHARING;
    } else if ("measurement_consent".equals(string)) {
      return MEASUREMENT_CONSENT;
    } else {
      return UNKNOWN;
    }
  }

  @Override
  public String toString() {
    switch (this) {
      case SESSION:
        return "session";
      case EVENT:
        return "event";
      case CLICK:
        return "click";
      case ATTRIBUTION:
        return "attribution";
      case INFO:
        return "info";
      case GDPR:
        return "gdpr";
      case DISABLE_THIRD_PARTY_SHARING:
        return "disable_third_party_sharing";
      case AD_REVENUE:
        return "ad_revenue";
      case SUBSCRIPTION:
        return "subscription";
      case THIRD_PARTY_SHARING:
        return "third_party_sharing";
      case MEASUREMENT_CONSENT:
        return "measurement_consent";
      default:
        return "unknown";
    }
  }
}
