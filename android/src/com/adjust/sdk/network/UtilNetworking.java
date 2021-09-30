package com.adjust.sdk.network;

import com.adjust.sdk.AdjustFactory;
import com.adjust.sdk.Constants;
import com.adjust.sdk.ILogger;

import org.json.JSONObject;

import java.io.IOException;
import java.net.URL;

import java.net.HttpURLConnection;

/**
 * Created by uerceg on 03/04/2017.
 */

public class UtilNetworking {
  private static String userAgent;

  private static ILogger getLogger() {
    return AdjustFactory.getLogger();
  }

  public static void setUserAgent(String userAgent) {
    UtilNetworking.userAgent = userAgent;
  }

  public interface IConnectionOptions {
    void applyConnectionOptions(HttpURLConnection connection, String clientSdk);
  }

  public static IConnectionOptions createDefaultConnectionOptions() {
    return new IConnectionOptions() {
      @Override
      public void applyConnectionOptions(
          final HttpURLConnection connection, final String clientSdk) {
        connection.setRequestProperty("Client-SDK", clientSdk);
        connection.setConnectTimeout(Constants.ONE_MINUTE);
        connection.setReadTimeout(Constants.ONE_MINUTE);

        if (userAgent != null) {
          connection.setRequestProperty("User-Agent", userAgent);
        }
      }
    };
  }

  public interface IHttpURLConnectionProvider {
    HttpURLConnection generateHttpURLConnection(URL url) throws IOException;
  }

  public static IHttpURLConnectionProvider createDefaultHttpURLConnectionProvider() {
    return new IHttpURLConnectionProvider() {
      @Override
      public HttpURLConnection generateHttpURLConnection(final URL url) throws IOException {
        return (HttpURLConnection) url.openConnection();
      }
    };
  }

  public static String extractJsonString(final JSONObject jsonObject, final String name) {
    // taken from JSONObject.optString(...) to add null fallback
    final Object object = jsonObject.opt(name);
    if (object instanceof String) {
      return (String) object;
    }
    if (object != null) {
      return object.toString();
    }
    return null;
  }

  public static Long extractJsonLong(final JSONObject jsonObject, final String name) {
    // taken from JSONObject.optLong(...) to add null fallback
    final Object object = jsonObject.opt(name);
    if (object instanceof Long) {
      return (Long) object;
    }
    if (object instanceof Number) {
      return ((Number) object).longValue();
    }
    if (object instanceof String) {
      try {
        return (long) Double.parseDouble((String) object);
      } catch (final NumberFormatException ignored) {
      }
    }
    return null;
  }
}
