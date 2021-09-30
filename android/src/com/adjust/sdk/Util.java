//
//  Util.java
//  Adjust
//
//  Created by Christian Wellenbrock on 2012-10-11.
//  Copyright (c) 2012-2014 adjust GmbH. All rights reserved.
//  See the file MIT-LICENSE for copying permission.
//

package com.adjust.sdk;

import android.content.ContentResolver;
import android.content.Context;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.net.ConnectivityManager;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkInfo;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Build;
import android.os.LocaleList;
import android.os.Looper;
import android.provider.Settings.Secure;
import android.telephony.TelephonyManager;
import android.text.TextUtils;

import com.adjust.sdk.scheduler.SingleThreadFutureScheduler;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.Closeable;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.NotSerializableException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.math.BigInteger;
import java.security.MessageDigest;
import java.text.DecimalFormat;
import java.text.DecimalFormatSymbols;
import java.text.SimpleDateFormat;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Random;
import java.util.UUID;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ScheduledFuture;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import static com.adjust.sdk.Constants.ENCODING;
import static com.adjust.sdk.Constants.MD5;
import static com.adjust.sdk.Constants.SHA1;
import static com.adjust.sdk.Constants.SHA256;

/**
 * Collects utility functions used by Adjust.
 */
public class Util {
    private static final String DATE_FORMAT = "yyyy-MM-dd'T'HH:mm:ss.SSS'Z'Z";
    private static final String fieldReadErrorMessage = "Unable to read '%s' field in migration device with message (%s)";
    public static final DecimalFormat SecondsDisplayFormat = newLocalDecimalFormat();
    public static final SimpleDateFormat dateFormatter = new SimpleDateFormat(DATE_FORMAT, Locale.US);

    // http://www.cs.umd.edu/~pugh/java/memoryModel/DoubleCheckedLocking.html
    private static volatile SingleThreadFutureScheduler playAdIdScheduler = null;

    private static ILogger getLogger() {
        return AdjustFactory.getLogger();
    }

    protected static String createUuid() {
        return UUID.randomUUID().toString();
    }

    private static DecimalFormat newLocalDecimalFormat() {
        DecimalFormatSymbols symbols = new DecimalFormatSymbols(Locale.US);
        return new DecimalFormat("0.0", symbols);
    }

    public static String quote(String string) {
        if (string == null) {
            return null;
        }

        Pattern pattern = Pattern.compile("\\s");
        Matcher matcher = pattern.matcher(string);
        if (!matcher.find()) {
            return string;
        }

        return Util.formatString("'%s'", string);
    }

    public static Object getAdvertisingInfoObject(final Context context, long timeoutMilli) {
        return runSyncInPlayAdIdSchedulerWithTimeout(context, new Callable<Object>() {
            @Override
            public Object call() {
                try {
                    return Reflection.getAdvertisingInfoObject(context);
                } catch (Exception e) {
                    return null;
                }
            }
        }, timeoutMilli);
    }

    public static String getPlayAdId(final Context context,
                                     final Object advertisingInfoObject,
                                     long timeoutMilli)
    {
        return runSyncInPlayAdIdSchedulerWithTimeout(context, new Callable<String>() {
            @Override
            public String call() {
                return Reflection.getPlayAdId(context, advertisingInfoObject);
            }
        }, timeoutMilli);
    }

    public static Boolean isPlayTrackingEnabled(final Context context,
                                               final Object advertisingInfoObject,
                                               long timeoutMilli)
    {
        return runSyncInPlayAdIdSchedulerWithTimeout(context, new Callable<Boolean>() {
            @Override
            public Boolean call() {
                return Reflection.isPlayTrackingEnabled(context, advertisingInfoObject);
            }
        }, timeoutMilli);
    }

    private static <R> R runSyncInPlayAdIdSchedulerWithTimeout(final Context context,
                                                               Callable<R> callable,
                                                               long timeoutMilli)
    {
        if (playAdIdScheduler == null) {
            synchronized (Util.class) {
                if (playAdIdScheduler == null) {
                    playAdIdScheduler = new SingleThreadFutureScheduler("PlayAdIdLibrary", true);
                }
            }
        }

        ScheduledFuture<R> playAdIdFuture = playAdIdScheduler.scheduleFutureWithReturn(callable, 0);

        try {
            return playAdIdFuture.get(timeoutMilli, TimeUnit.MILLISECONDS);
        } catch (ExecutionException e) {
        } catch (InterruptedException e) {
        } catch (TimeoutException e) {
        }

        return null;
    }

    public static void runInBackground(Runnable command) {
        if (Looper.myLooper() != Looper.getMainLooper()) {
            command.run();
            return;
        }
        new AsyncTask<Object,Void,Void>() {
            @Override
            protected Void doInBackground(Object... params) {
                Runnable command = (Runnable)params[0];
                command.run();
                return null;
            }
        }.execute((Object)command);
    }

    public static void getGoogleAdId(Context context, final OnDeviceIdsRead onDeviceIdRead) {
        ILogger logger = AdjustFactory.getLogger();
        if (Looper.myLooper() != Looper.getMainLooper()) {
            logger.debug("GoogleAdId being read in the background");

            String GoogleAdId = Util.getGoogleAdId(context);

            logger.debug("GoogleAdId read " + GoogleAdId);
            onDeviceIdRead.onGoogleAdIdRead(GoogleAdId);
            return;
        }

        logger.debug("GoogleAdId being read in the foreground");
        new AsyncTask<Context,Void,String>() {
            @Override
            protected String doInBackground(Context... params) {
                ILogger logger = AdjustFactory.getLogger();
                Context innerContext = params[0];
                String innerResult = Util.getGoogleAdId(innerContext);
                logger.debug("GoogleAdId read " + innerResult);
                return innerResult;
            }

            @Override
            protected void onPostExecute(String playAdiId) {
                ILogger logger = AdjustFactory.getLogger();
                onDeviceIdRead.onGoogleAdIdRead(playAdiId);
            }
        }.execute(context);
    }

    private static String getGoogleAdId(Context context) {
        String googleAdId = null;
        try {
            GooglePlayServicesClient.GooglePlayServicesInfo gpsInfo =
                    GooglePlayServicesClient.getGooglePlayServicesInfo(context,
                            Constants.ONE_SECOND * 11);
            if (gpsInfo != null) {
                googleAdId = gpsInfo.getGpsAdid();
            }
        } catch (Exception e) {
        }
        if (googleAdId == null) {
            Object advertisingInfoObject = Util.getAdvertisingInfoObject(
                    context, Constants.ONE_SECOND * 11);

            if (advertisingInfoObject != null) {
                googleAdId = Util.getPlayAdId(context, advertisingInfoObject, Constants.ONE_SECOND);
            }
        }

        return googleAdId;
    }

    public static String getMacAddress(Context context) {
        return MacAddressUtil.getMacAddress(context);
    }

    public static String getAndroidId(Context context) {
        return AndroidIdUtil.getAndroidId(context);
    }

    public static <T> T readObject(Context context, String filename, String objectName, Class<T> type) {
        Closeable closable = null;
        T object = null;
        try {
            FileInputStream inputStream = context.openFileInput(filename);
            closable = inputStream;

            BufferedInputStream bufferedStream = new BufferedInputStream(inputStream);
            closable = bufferedStream;

            ObjectInputStream objectStream = new ObjectInputStream(bufferedStream);
            closable = objectStream;

            try {
                object = type.cast(objectStream.readObject());
                getLogger().debug("Read %s: %s", objectName, object);
            } catch (ClassNotFoundException e) {
                getLogger().error("Failed to find %s class (%s)", objectName, e.getMessage());
            } catch (ClassCastException e) {
                getLogger().error("Failed to cast %s object (%s)", objectName, e.getMessage());
            } catch (Exception e) {
                getLogger().error("Failed to read %s object (%s)", objectName, e.getMessage());
            }
        } catch (FileNotFoundException e) {
            getLogger().debug("%s file not found", objectName);
        } catch (Exception e) {
            getLogger().error("Failed to open %s file for reading (%s)", objectName, e);
        }
        try {
            if (closable != null) {
                closable.close();
            }
        } catch (Exception e) {
            getLogger().error("Failed to close %s file for reading (%s)", objectName, e);
        }

        return object;
    }

    public static <T> void writeObject(T object, Context context, String filename, String objectName) {
        Closeable closable = null;
        try {
            FileOutputStream outputStream = context.openFileOutput(filename, Context.MODE_PRIVATE);
            closable = outputStream;

            BufferedOutputStream bufferedStream = new BufferedOutputStream(outputStream);
            closable = bufferedStream;

            ObjectOutputStream objectStream = new ObjectOutputStream(bufferedStream);
            closable = objectStream;

            try {
                objectStream.writeObject(object);

                getLogger().debug("Wrote %s: %s", objectName, object);
            } catch (NotSerializableException e) {
                getLogger().error("Failed to serialize %s", objectName);
            }
        } catch (Exception e) {
            getLogger().error("Failed to open %s for writing (%s)", objectName, e);
        }
        try {
            if (closable != null) {
                closable.close();
            }
        } catch (Exception e) {
            getLogger().error("Failed to close %s file for writing (%s)", objectName, e);
        }
    }

    public static boolean checkPermission(Context context, String permission) {
        try {
            int result = context.checkCallingOrSelfPermission(permission);
            return result == PackageManager.PERMISSION_GRANTED;
        } catch (Exception e) {
            getLogger().debug("Unable to check permission '%s' with message (%s)", permission, e.getMessage());
            return false;
        }
    }

    public static String readStringField(ObjectInputStream.GetField fields, String name, String defaultValue) {
        return readObjectField(fields, name, defaultValue);
    }

    public static <T> T readObjectField(ObjectInputStream.GetField fields, String name, T defaultValue) {
        try {
            return (T) fields.get(name, defaultValue);
        } catch (Exception e) {
            getLogger().debug(fieldReadErrorMessage, name, e.getMessage());
            return defaultValue;
        }
    }

    public static boolean readBooleanField(ObjectInputStream.GetField fields, String name, boolean defaultValue) {
        try {
            return fields.get(name, defaultValue);
        } catch (Exception e) {
            getLogger().debug(fieldReadErrorMessage, name, e.getMessage());
            return defaultValue;
        }
    }

    public static int readIntField(ObjectInputStream.GetField fields, String name, int defaultValue) {
        try {
            return fields.get(name, defaultValue);
        } catch (Exception e) {
            getLogger().debug(fieldReadErrorMessage, name, e.getMessage());
            return defaultValue;
        }
    }

    public static long readLongField(ObjectInputStream.GetField fields, String name, long defaultValue) {
        try {
            return fields.get(name, defaultValue);
        } catch (Exception e) {
            getLogger().debug(fieldReadErrorMessage, name, e.getMessage());
            return defaultValue;
        }
    }

    public static boolean equalObject(Object first, Object second) {
        if (first == null || second == null) {
            return first == null && second == null;
        }
        return first.equals(second);
    }

    public static boolean equalsDouble(Double first, Double second) {
        if (first == null || second == null) {
            return first == null && second == null;
        }
        return Double.doubleToLongBits(first) == Double.doubleToLongBits(second);
    }

    public static boolean equalString(String first, String second) {
        return equalObject(first, second);
    }

    public static boolean equalEnum(Enum first, Enum second) {
        return equalObject(first, second);
    }

    public static boolean equalLong(Long first, Long second) {
        return equalObject(first, second);
    }

    public static boolean equalInt(Integer first, Integer second) {
        return equalObject(first, second);
    }

    public static boolean equalBoolean(Boolean first, Boolean second) {
        return equalObject(first, second);
    }

    public static int hashBoolean(Boolean value) {
        if (value == null) {
            return 0;
        }
        return value.hashCode();
    }

    public static int hashLong(Long value) {
        if (value == null) {
            return 0;
        }
        return value.hashCode();
    }

    public static int hashDouble(Double value) {
        if (value == null) {
            return 0;
        }
        return value.hashCode();
    }

    public static int hashString(String value) {
        if (value == null) {
            return 0;
        }
        return value.hashCode();
    }

    public static int hashEnum(Enum value) {
        if (value == null) {
            return 0;
        }
        return value.hashCode();
    }

    public static int hashObject(Object value) {
        if (value == null) {
            return 0;
        }
        return value.hashCode();
    }

    public static String sha1(final String text) {
        return hash(text, SHA1);
    }

    public static String sha256(final String text) {
        return hash(text, SHA256);
    }

    public static String md5(final String text) {
        return hash(text, MD5);
    }

    public static String hash(final String text, final String method) {
        String hashString = null;
        try {
            final byte[] bytes = text.getBytes(ENCODING);
            final MessageDigest mesd = MessageDigest.getInstance(method);
            mesd.update(bytes, 0, bytes.length);
            final byte[] hash = mesd.digest();
            hashString = convertToHex(hash);
        } catch (Exception e) {
        }
        return hashString;
    }

    public static String convertToHex(final byte[] bytes) {
        final BigInteger bigInt = new BigInteger(1, bytes);
        final String formatString = "%0" + (bytes.length << 1) + "x";
        return Util.formatString(formatString, bigInt);
    }

    public static String[] getSupportedAbis() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            return Build.SUPPORTED_ABIS;
        }
        return null;
    }

    public static String getCpuAbi() {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.LOLLIPOP) {
            return Build.CPU_ABI;
        }
        return null;
    }

    public static String getReasonString(String message, Throwable throwable) {
        if (throwable != null) {
            return Util.formatString("%s: %s", message, throwable);
        } else {
            return Util.formatString("%s", message);
        }
    }

    public static long getWaitingTime(int retries, BackoffStrategy backoffStrategy) {
        if (retries < backoffStrategy.minRetries) {
            return 0;
        }
        // start with expon 0
        int expon = retries - backoffStrategy.minRetries;
        // get the exponential Time from the power of 2: 1, 2, 4, 8, 16, ... * times the multiplier
        long exponentialTime = ((long) Math.pow(2, expon)) * backoffStrategy.milliSecondMultiplier;
        // limit the maximum allowed time to wait
        long ceilingTime = Math.min(exponentialTime, backoffStrategy.maxWait);
        // get the random range
        double randomDouble = randomInRange(backoffStrategy.minRange, backoffStrategy.maxRange);
        // apply jitter factor
        double waitingTime =  ceilingTime * randomDouble;
        return (long)waitingTime;
    }

    private static double randomInRange(double minRange, double maxRange) {
        Random random = new Random();
        double range = maxRange - minRange;
        double scaled = random.nextDouble() * range;
        double shifted = scaled + minRange;
        return shifted;
    }

    public static boolean isValidParameter(String attribute, String attributeType, String parameterName) {
        if (attribute == null) {
            getLogger().error("%s parameter %s is missing", parameterName, attributeType);
            return false;
        }
        if (attribute.equals("")) {
            getLogger().error("%s parameter %s is empty", parameterName, attributeType);
            return false;
        }

        return true;
    }

    public static Map<String, String> mergeParameters(Map<String, String> target,
                                                      Map<String, String> source,
                                                      String parameterName) {
        if (target == null) {
            return source;
        }
        if (source == null) {
            return target;
        }
        Map<String, String> mergedParameters = new HashMap<String, String>(target);
        ILogger logger = getLogger();
        for (Map.Entry<String, String> parameterSourceEntry : source.entrySet()) {
            String oldValue = mergedParameters.put(parameterSourceEntry.getKey(), parameterSourceEntry.getValue());
            if (oldValue != null) {
                logger.warn("Key %s with value %s from %s parameter was replaced by value %s",
                        parameterSourceEntry.getKey(),
                        oldValue,
                        parameterName,
                        parameterSourceEntry.getValue());
            }
        }
        return mergedParameters;
    }

    public static Locale getLocale(Configuration configuration) {
        // Configuration.getLocales() added as of API 24.
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            LocaleList localesList = configuration.getLocales();
            if (localesList != null && !localesList.isEmpty()) {
                return localesList.get(0);
            }
        }

        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.N) {
            return configuration.locale;
        }
        return null;
    }

    public static String getFireAdvertisingId(ContentResolver contentResolver) {
        if (contentResolver == null) {
            return null;
        }
        try {
            // get advertising
            return Secure.getString(contentResolver, "advertising_id");
        } catch (Exception ex) {
            // not supported
        }
        return null;
    }

    public static Boolean getFireTrackingEnabled(ContentResolver contentResolver) {
        try {
            // get user's tracking preference
            return Secure.getInt(contentResolver, "limit_ad_tracking") == 0;
        } catch (Exception ex) {
            // not supported
        }
        return null;
    }

    public static int getConnectivityType(Context context) {
        try {
            ConnectivityManager cm = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);

            if (cm == null) {
                return -1;
            }

            // for api 22 or lower, still need to get raw type
            if (Build.VERSION.SDK_INT < Build.VERSION_CODES.M) {
                NetworkInfo activeNetwork = cm.getActiveNetworkInfo();
                return activeNetwork.getType();
            }

            // .getActiveNetwork() is only available from api 23
            Network activeNetwork = cm.getActiveNetwork();
            if (activeNetwork == null) {
                return -1;
            }

            NetworkCapabilities activeNetworkCapabilities = cm.getNetworkCapabilities(activeNetwork);
            if (activeNetworkCapabilities == null) {
                return -1;
            }

            // check each network capability available from api 23
            if (activeNetworkCapabilities.hasTransport(NetworkCapabilities.TRANSPORT_WIFI)) {
                return NetworkCapabilities.TRANSPORT_WIFI;
            }
            if (activeNetworkCapabilities.hasTransport(NetworkCapabilities.TRANSPORT_CELLULAR)) {
                return NetworkCapabilities.TRANSPORT_CELLULAR;
            }
            if (activeNetworkCapabilities.hasTransport(NetworkCapabilities.TRANSPORT_ETHERNET)) {
                return NetworkCapabilities.TRANSPORT_ETHERNET;
            }
            if (activeNetworkCapabilities.hasTransport(NetworkCapabilities.TRANSPORT_VPN)) {
                return NetworkCapabilities.TRANSPORT_VPN;
            }
            if (activeNetworkCapabilities.hasTransport(NetworkCapabilities.TRANSPORT_BLUETOOTH)) {
                return NetworkCapabilities.TRANSPORT_BLUETOOTH;
            }

            // only after api 26, that more transport capabilities were added
            if (Build.VERSION.SDK_INT < Build.VERSION_CODES.O) {
                return -1;
            }

            if (activeNetworkCapabilities.hasTransport(NetworkCapabilities.TRANSPORT_WIFI_AWARE)) {
                return NetworkCapabilities.TRANSPORT_WIFI_AWARE;
            }

            // and then after api 27, that more transport capabilities were added
            if (Build.VERSION.SDK_INT < Build.VERSION_CODES.O_MR1) {
                return -1;
            }

            if (activeNetworkCapabilities.hasTransport(NetworkCapabilities.TRANSPORT_LOWPAN)) {
                return NetworkCapabilities.TRANSPORT_LOWPAN;
            }
        } catch (Exception e) {
            getLogger().warn("Couldn't read connectivity type (%s)", e.getMessage());
        }

        return -1;
    }

    public static int getNetworkType(Context context) {
        int networkType = -1; // default value that will not be send

        try {
            TelephonyManager teleMan =
                    (TelephonyManager) context.getSystemService(Context.TELEPHONY_SERVICE);
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
                networkType = teleMan.getDataNetworkType();
            } else {
                networkType = teleMan.getNetworkType();
            }
        } catch (Exception e) {
            getLogger().warn("Couldn't read network type (%s)", e.getMessage());
        }

        return networkType;
    }

    public static String getMcc(Context context) {
        try {
            TelephonyManager tel = (TelephonyManager) context.getSystemService(Context.TELEPHONY_SERVICE);
            String networkOperator = tel.getNetworkOperator();

            if (TextUtils.isEmpty(networkOperator)) {
                AdjustFactory.getLogger().warn("Couldn't receive networkOperator string to read MCC");
                return null;
            }
            return networkOperator.substring(0, 3);
        } catch (Exception ex) {
            AdjustFactory.getLogger().warn("Couldn't return mcc");
            return null;
        }
    }

    public static String getMnc(Context context) {
        try {
            TelephonyManager tel = (TelephonyManager) context.getSystemService(Context.TELEPHONY_SERVICE);
            String networkOperator = tel.getNetworkOperator();

            if (TextUtils.isEmpty(networkOperator)) {
                AdjustFactory.getLogger().warn("Couldn't receive networkOperator string to read MNC");
                return null;
            }
            return networkOperator.substring(3);
        } catch (Exception ex) {
            AdjustFactory.getLogger().warn("Couldn't return mnc");
            return null;
        }
    }

    public static String formatString(String format, Object... args) {
        return String.format(Locale.US, format, args);
    }

    public static boolean hasRootCause(Exception ex) {
        StringWriter sw = new StringWriter();
        PrintWriter pw = new PrintWriter(sw);
        ex.printStackTrace(pw);
        String sStackTrace = sw.toString(); // stack trace as a string

        return sStackTrace.contains("Caused by:");
    }

    public static String getRootCause(Exception ex) {
        if (!hasRootCause(ex)) {
            return null;
        }

        StringWriter sw = new StringWriter();
        PrintWriter pw = new PrintWriter(sw);
        ex.printStackTrace(pw);
        String sStackTrace = sw.toString(); // stack trace as a string

        int startOccuranceOfRootCause = sStackTrace.indexOf("Caused by:");
        int endOccuranceOfRootCause = sStackTrace.indexOf("\n", startOccuranceOfRootCause);
        return sStackTrace.substring(startOccuranceOfRootCause, endOccuranceOfRootCause);
    }

    private static String getSdkPrefix(final String clientSdk) {
        if (clientSdk == null) {
            return null;
        }
        if (!clientSdk.contains("@")) {
            return null;
        }

        String[] splitted = clientSdk.split("@");
        if (splitted == null) {
            return null;
        }
        if (splitted.length != 2) {
            return null;
        }

        return splitted[0];
    }

    public static String getSdkPrefixPlatform(final String clientSdk) {
        String sdkPrefix = getSdkPrefix(clientSdk);
        if (sdkPrefix == null) {
            return null;
        }

        String[] splitted = sdkPrefix.split("\\d+", 2);
        if (splitted == null) {
            return null;
        }
        if (splitted.length == 0) {
            return null;
        }

        return splitted[0];
    }

    public static boolean isUrlFilteredOut(Uri url) {
        if (url == null) {
            return true;
        }

        String urlString = url.toString();

        if (urlString == null || urlString.length() == 0) {
            return true;
        }

        // Url with FB credentials to be filtered out
        if (urlString.matches(Constants.FB_AUTH_REGEX)) {
            return true;
        }

        return false;
    }

    public static String getSdkVersion() {
        return Constants.CLIENT_SDK;
    }

    public static boolean resolveContentProvider(final Context applicationContext,
                                                 final String authority) {
        try {
            return (applicationContext.getPackageManager()
                    .resolveContentProvider(authority, 0) != null);

        } catch (Exception e) {
            return false;
        }
    }

    public static boolean isEqualReferrerDetails(final ReferrerDetails referrerDetails,
                                                 final String referrerApi,
                                                 final ActivityState activityState) {
        if (referrerApi.equals(Constants.REFERRER_API_GOOGLE)) {
            return isEqualGoogleReferrerDetails(referrerDetails, activityState);
        } else if (referrerApi.equals(Constants.REFERRER_API_HUAWEI)) {
            return isEqualHuaweiReferrerDetails(referrerDetails, activityState);
        }

        return false;
    }

    private static boolean isEqualGoogleReferrerDetails(final ReferrerDetails referrerDetails,
                                                       final ActivityState activityState) {
        return referrerDetails.referrerClickTimestampSeconds == activityState.clickTime
                && referrerDetails.installBeginTimestampSeconds == activityState.installBegin
                && referrerDetails.referrerClickTimestampServerSeconds == activityState.clickTimeServer
                && referrerDetails.installBeginTimestampServerSeconds == activityState.installBeginServer
                && Util.equalString(referrerDetails.installReferrer, activityState.installReferrer)
                && Util.equalString(referrerDetails.installVersion, activityState.installVersion)
                && Util.equalBoolean(referrerDetails.googlePlayInstant, activityState.googlePlayInstant) ;
    }

    private static boolean isEqualHuaweiReferrerDetails(final ReferrerDetails referrerDetails,
                                                       final ActivityState activityState) {
        return referrerDetails.referrerClickTimestampSeconds == activityState.clickTimeHuawei
                && referrerDetails.installBeginTimestampSeconds == activityState.installBeginHuawei
                && Util.equalString(referrerDetails.installReferrer, activityState.installReferrerHuawei);
    }
}