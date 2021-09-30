package com.adjust.sdk;

import android.content.Context;

import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.Map;

public class Reflection {
  public static Object getAdvertisingInfoObject(Context context) throws Exception {
    return invokeStaticMethod("com.google.android.gms.ads.identifier.AdvertisingIdClient",
        "getAdvertisingIdInfo", new Class[] {Context.class}, context);
  }

  static Map<String, String> getImeiParameters(Context context, ILogger logger) {
    Object nonPlayParameters = null;
    try {
      nonPlayParameters = invokeStaticMethod("com.adjust.sdk.imei.Util", "getImeiParameters",
          new Class[] {Context.class, ILogger.class}, context, logger);
      Class<Map<String, String>> stringStringMapClass =
          (Class<Map<String, String>>) (Class) Map.class;
      if (nonPlayParameters != null && stringStringMapClass.isInstance(nonPlayParameters)) {
        return (Map<String, String>) nonPlayParameters;
      }
    } catch (Exception e) {
    }
    return null;
  }

  static Map<String, String> getOaidParameters(Context context, ILogger logger) {
    Object oaidParameters = null;
    try {
      oaidParameters = invokeStaticMethod("com.adjust.sdk.oaid.Util", "getOaidParameters",
          new Class[] {Context.class, ILogger.class}, context, logger);
      Class<Map<String, String>> stringStringMapClass =
          (Class<Map<String, String>>) (Class) Map.class;
      if (oaidParameters != null && stringStringMapClass.isInstance(oaidParameters)) {
        return (Map<String, String>) oaidParameters;
      }
    } catch (Exception e) {
    }
    return null;
  }

  public static String getPlayAdId(Context context, Object AdvertisingInfoObject) {
    try {
      String playAdid = (String) invokeInstanceMethod(AdvertisingInfoObject, "getId", null);
      return playAdid;
    } catch (Throwable t) {
      return null;
    }
  }

  public static Boolean isPlayTrackingEnabled(Context context, Object AdvertisingInfoObject) {
    try {
      Boolean isLimitedTrackingEnabled =
          (Boolean) invokeInstanceMethod(AdvertisingInfoObject, "isLimitAdTrackingEnabled", null);
      Boolean isPlayTrackingEnabled =
          (isLimitedTrackingEnabled == null ? null : !isLimitedTrackingEnabled);
      return isPlayTrackingEnabled;
    } catch (Throwable t) {
      return null;
    }
  }

  public static Class forName(String className) {
    try {
      Class classObject = Class.forName(className);
      return classObject;
    } catch (Throwable t) {
      return null;
    }
  }

  public static Object createDefaultInstance(String className) {
    Class classObject = forName(className);
    if (classObject == null) {
      return null;
    }

    Object instance = createDefaultInstance(classObject);
    return instance;
  }

  public static Object createDefaultInstance(Class classObject) {
    try {
      Object instance = classObject.newInstance();
      return instance;
    } catch (Throwable t) {
      return null;
    }
  }

  public static Object createInstance(String className, Class[] cArgs, Object... args) {
    try {
      Class classObject = Class.forName(className);
      @SuppressWarnings("unchecked") Constructor constructor = classObject.getConstructor(cArgs);
      Object instance = constructor.newInstance(args);
      return instance;
    } catch (Throwable t) {
      return null;
    }
  }

  public static Object invokeStaticMethod(
      String className, String methodName, Class[] cArgs, Object... args) throws Exception {
    Class classObject = Class.forName(className);
    return invokeMethod(classObject, methodName, null, cArgs, args);
  }

  public static Object invokeInstanceMethod(
      Object instance, String methodName, Class[] cArgs, Object... args) throws Exception {
    Class classObject = instance.getClass();
    return invokeMethod(classObject, methodName, instance, cArgs, args);
  }

  public static Object invokeMethod(Class classObject, String methodName, Object instance,
      Class[] cArgs, Object... args) throws Exception {
    @SuppressWarnings("unchecked") Method methodObject = classObject.getMethod(methodName, cArgs);
    if (methodObject == null) {
      return null;
    }

    Object resultObject = methodObject.invoke(instance, args);
    return resultObject;
  }

  public static Object readField(String className, String fieldName) throws Exception {
    return readField(className, fieldName, null);
  }

  public static Object readField(String className, String fieldName, Object instance)
      throws Exception {
    Class classObject = forName(className);
    if (classObject == null) {
      return null;
    }
    Field fieldObject = classObject.getField(fieldName);
    if (fieldObject == null) {
      return null;
    }
    return fieldObject.get(instance);
  }
}
