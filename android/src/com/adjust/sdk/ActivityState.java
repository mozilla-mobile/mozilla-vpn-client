//
//  ActivityState.java
//  Adjust
//
//  Created by Christian Wellenbrock on 2013-06-25.
//  Copyright (c) 2013 adjust GmbH. All rights reserved.
//  See the file MIT-LICENSE for copying permission.
//

package com.adjust.sdk;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectInputStream.GetField;
import java.io.ObjectOutputStream;
import java.io.ObjectStreamField;
import java.io.Serializable;
import java.util.Calendar;
import java.util.LinkedList;

public class ActivityState implements Serializable, Cloneable {
  private static final long serialVersionUID = 9039439291143138148L;
  private static final int ORDER_ID_MAXCOUNT = 10;
  private transient ILogger logger;
  private static final ObjectStreamField[] serialPersistentFields = {
      new ObjectStreamField("uuid", String.class),
      new ObjectStreamField("enabled", boolean.class),
      new ObjectStreamField("isGdprForgotten", boolean.class),
      new ObjectStreamField("isThirdPartySharingDisabled", boolean.class),
      new ObjectStreamField("askingAttribution", boolean.class),
      new ObjectStreamField("eventCount", int.class),
      new ObjectStreamField("sessionCount", int.class),
      new ObjectStreamField("subsessionCount", int.class),
      new ObjectStreamField("sessionLength", long.class),
      new ObjectStreamField("timeSpent", long.class),
      new ObjectStreamField("lastActivity", long.class),
      new ObjectStreamField("lastInterval", long.class),
      new ObjectStreamField("updatePackages", boolean.class),
      new ObjectStreamField("orderIds", (Class<LinkedList<String>>) (Class) LinkedList.class),
      new ObjectStreamField("pushToken", String.class),
      new ObjectStreamField("adid", String.class),
      new ObjectStreamField("clickTime", long.class),
      new ObjectStreamField("installBegin", long.class),
      new ObjectStreamField("installReferrer", String.class),
      new ObjectStreamField("googlePlayInstant", Boolean.class),
      new ObjectStreamField("clickTimeServer", long.class),
      new ObjectStreamField("installBeginServer", long.class),
      new ObjectStreamField("installVersion", String.class),
      new ObjectStreamField("clickTimeHuawei", long.class),
      new ObjectStreamField("installBeginHuawei", long.class),
      new ObjectStreamField("installReferrerHuawei", String.class),
  };

  // persistent data
  protected String uuid;
  protected boolean enabled;
  protected boolean isGdprForgotten;
  protected boolean isThirdPartySharingDisabled;
  protected boolean askingAttribution;

  // global counters
  protected int eventCount;
  protected int sessionCount;

  // session attributes
  protected int subsessionCount;
  protected long sessionLength; // all durations in milliseconds
  protected long timeSpent;
  protected long lastActivity; // all times in milliseconds since 1970

  protected long lastInterval;

  protected boolean updatePackages;

  protected LinkedList<String> orderIds;

  protected String pushToken;
  protected String adid;

  protected long clickTime;
  protected long installBegin;
  protected String installReferrer;
  protected Boolean googlePlayInstant;
  protected long clickTimeServer;
  protected long installBeginServer;
  protected String installVersion;

  protected long clickTimeHuawei;
  protected long installBeginHuawei;
  protected String installReferrerHuawei;

  protected ActivityState() {
    logger = AdjustFactory.getLogger();
    // create UUID for new devices
    uuid = Util.createUuid();
    enabled = true;
    isGdprForgotten = false;
    isThirdPartySharingDisabled = false;
    askingAttribution = false;
    eventCount = 0; // no events yet
    sessionCount = 0; // the first session just started
    subsessionCount = -1; // we don't know how many subsessions this first  session will have
    sessionLength = -1; // same for session length and time spent
    timeSpent = -1; // this information will be collected and attached to the next session
    lastActivity = -1;
    lastInterval = -1;
    updatePackages = false;
    orderIds = null;
    pushToken = null;
    adid = null;
    clickTime = 0;
    installBegin = 0;
    installReferrer = null;
    googlePlayInstant = null;
    clickTimeServer = 0;
    installBeginServer = 0;
    installVersion = null;
    clickTimeHuawei = 0;
    installBeginHuawei = 0;
    installReferrerHuawei = null;
  }

  protected void resetSessionAttributes(long now) {
    subsessionCount = 1; // first subsession
    sessionLength = 0; // no session length yet
    timeSpent = 0; // no time spent yet
    lastActivity = now;
    lastInterval = -1;
  }

  protected void addOrderId(String orderId) {
    if (orderIds == null) {
      orderIds = new LinkedList<String>();
    }

    if (orderIds.size() >= ORDER_ID_MAXCOUNT) {
      orderIds.removeLast();
    }
    orderIds.addFirst(orderId);
  }

  protected boolean findOrderId(String orderId) {
    if (orderIds == null) {
      return false;
    }
    return orderIds.contains(orderId);
  }

  @Override
  public String toString() {
    return Util.formatString("ec:%d sc:%d ssc:%d sl:%.1f ts:%.1f la:%s uuid:%s", eventCount,
        sessionCount, subsessionCount, sessionLength / 1000.0, timeSpent / 1000.0,
        stamp(lastActivity), uuid);
  }

  @Override
  public boolean equals(Object other) {
    if (other == this)
      return true;
    if (other == null)
      return false;
    if (getClass() != other.getClass())
      return false;
    ActivityState otherActivityState = (ActivityState) other;

    if (!Util.equalString(uuid, otherActivityState.uuid))
      return false;
    if (!Util.equalBoolean(enabled, otherActivityState.enabled))
      return false;
    if (!Util.equalBoolean(isGdprForgotten, otherActivityState.isGdprForgotten))
      return false;
    if (!Util.equalBoolean(
            isThirdPartySharingDisabled, otherActivityState.isThirdPartySharingDisabled))
      return false;
    if (!Util.equalBoolean(askingAttribution, otherActivityState.askingAttribution))
      return false;
    if (!Util.equalInt(eventCount, otherActivityState.eventCount))
      return false;
    if (!Util.equalInt(sessionCount, otherActivityState.sessionCount))
      return false;
    if (!Util.equalInt(subsessionCount, otherActivityState.subsessionCount))
      return false;
    if (!Util.equalLong(sessionLength, otherActivityState.sessionLength))
      return false;
    if (!Util.equalLong(timeSpent, otherActivityState.timeSpent))
      return false;
    if (!Util.equalLong(lastInterval, otherActivityState.lastInterval))
      return false;
    if (!Util.equalBoolean(updatePackages, otherActivityState.updatePackages))
      return false;
    if (!Util.equalObject(orderIds, otherActivityState.orderIds))
      return false;
    if (!Util.equalString(pushToken, otherActivityState.pushToken))
      return false;
    if (!Util.equalString(adid, otherActivityState.adid))
      return false;
    if (!Util.equalLong(clickTime, otherActivityState.clickTime))
      return false;
    if (!Util.equalLong(installBegin, otherActivityState.installBegin))
      return false;
    if (!Util.equalString(installReferrer, otherActivityState.installReferrer))
      return false;
    if (!Util.equalBoolean(googlePlayInstant, otherActivityState.googlePlayInstant))
      return false;
    if (!Util.equalLong(clickTimeServer, otherActivityState.clickTimeServer))
      return false;
    if (!Util.equalLong(installBeginServer, otherActivityState.installBeginServer))
      return false;
    if (!Util.equalString(installVersion, otherActivityState.installVersion))
      return false;
    if (!Util.equalLong(clickTimeHuawei, otherActivityState.clickTimeHuawei))
      return false;
    if (!Util.equalLong(installBeginHuawei, otherActivityState.installBeginHuawei))
      return false;
    if (!Util.equalString(installReferrerHuawei, otherActivityState.installReferrerHuawei))
      return false;
    return true;
  }

  @Override
  public int hashCode() {
    int hashCode = 17;
    hashCode = 37 * hashCode + Util.hashString(uuid);
    hashCode = 37 * hashCode + Util.hashBoolean(enabled);
    hashCode = 37 * hashCode + Util.hashBoolean(isGdprForgotten);
    hashCode = 37 * hashCode + Util.hashBoolean(isThirdPartySharingDisabled);
    hashCode = 37 * hashCode + Util.hashBoolean(askingAttribution);
    hashCode = 37 * hashCode + eventCount;
    hashCode = 37 * hashCode + sessionCount;
    hashCode = 37 * hashCode + subsessionCount;
    hashCode = 37 * hashCode + Util.hashLong(sessionLength);
    hashCode = 37 * hashCode + Util.hashLong(timeSpent);
    hashCode = 37 * hashCode + Util.hashLong(lastInterval);
    hashCode = 37 * hashCode + Util.hashBoolean(updatePackages);
    hashCode = 37 * hashCode + Util.hashObject(orderIds);
    hashCode = 37 * hashCode + Util.hashString(pushToken);
    hashCode = 37 * hashCode + Util.hashString(adid);
    hashCode = 37 * hashCode + Util.hashLong(clickTime);
    hashCode = 37 * hashCode + Util.hashLong(installBegin);
    hashCode = 37 * hashCode + Util.hashString(installReferrer);
    hashCode = 37 * hashCode + Util.hashBoolean(googlePlayInstant);
    hashCode = 37 * hashCode + Util.hashLong(clickTimeServer);
    hashCode = 37 * hashCode + Util.hashLong(installBeginServer);
    hashCode = 37 * hashCode + Util.hashString(installVersion);
    hashCode = 37 * hashCode + Util.hashLong(clickTimeHuawei);
    hashCode = 37 * hashCode + Util.hashLong(installBeginHuawei);
    hashCode = 37 * hashCode + Util.hashString(installReferrerHuawei);
    return hashCode;
  }

  private void readObject(ObjectInputStream stream) throws IOException, ClassNotFoundException {
    GetField fields = stream.readFields();

    eventCount = Util.readIntField(fields, "eventCount", 0);
    sessionCount = Util.readIntField(fields, "sessionCount", 0);
    subsessionCount = Util.readIntField(fields, "subsessionCount", -1);
    sessionLength = Util.readLongField(fields, "sessionLength", -1l);
    timeSpent = Util.readLongField(fields, "timeSpent", -1l);
    lastActivity = Util.readLongField(fields, "lastActivity", -1l);
    lastInterval = Util.readLongField(fields, "lastInterval", -1l);

    // new fields
    uuid = Util.readStringField(fields, "uuid", null);
    enabled = Util.readBooleanField(fields, "enabled", true);
    isGdprForgotten = Util.readBooleanField(fields, "isGdprForgotten", false);
    isThirdPartySharingDisabled =
        Util.readBooleanField(fields, "isThirdPartySharingDisabled", false);
    askingAttribution = Util.readBooleanField(fields, "askingAttribution", false);

    updatePackages = Util.readBooleanField(fields, "updatePackages", false);
    orderIds = Util.readObjectField(fields, "orderIds", null);
    pushToken = Util.readStringField(fields, "pushToken", null);
    adid = Util.readStringField(fields, "adid", null);

    clickTime = Util.readLongField(fields, "clickTime", -1l);
    installBegin = Util.readLongField(fields, "installBegin", -1l);
    installReferrer = Util.readStringField(fields, "installReferrer", null);
    googlePlayInstant = Util.readObjectField(fields, "googlePlayInstant", null);
    clickTimeServer = Util.readLongField(fields, "clickTimeServer", -1l);
    installBeginServer = Util.readLongField(fields, "installBeginServer", -1l);
    installVersion = Util.readStringField(fields, "installVersion", null);

    clickTimeHuawei = Util.readLongField(fields, "clickTimeHuawei", -1l);
    installBeginHuawei = Util.readLongField(fields, "installBeginHuawei", -1l);
    installReferrerHuawei = Util.readStringField(fields, "installReferrerHuawei", null);

    // create UUID for migrating devices
    if (uuid == null) {
      uuid = Util.createUuid();
    }
  }

  private void writeObject(ObjectOutputStream stream) throws IOException {
    stream.defaultWriteObject();
  }

  private static String stamp(long dateMillis) {
    Calendar calendar = Calendar.getInstance();
    calendar.setTimeInMillis(dateMillis);
    return Util.formatString(
        "%02d:%02d:%02d", calendar.HOUR_OF_DAY, calendar.MINUTE, calendar.SECOND);
  }
}
