package com.adjust.sdk;

import android.net.Uri;

import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public final class AdjustLinkResolution {
  public interface AdjustLinkResolutionCallback {
    void resolvedLinkCallback(Uri resolvedLink);
  }

  // http://www.cs.umd.edu/~pugh/java/memoryModel/DoubleCheckedLocking.html
  private static volatile ExecutorService executor;

  private static final int maxRecursions = 10;
  private static final String[] expectedUrlHostSuffixArray = {"adjust.com", "adj.st", "go.link"};

  private AdjustLinkResolution() {}

  public static void resolveLink(final String url, final String[] resolveUrlSuffixArray,
      final AdjustLinkResolutionCallback adjustLinkResolutionCallback) {
    if (adjustLinkResolutionCallback == null) {
      return;
    }

    if (url == null) {
      adjustLinkResolutionCallback.resolvedLinkCallback(null);
      return;
    }

    URL originalURL = null;
    try {
      originalURL = new URL(url);
    } catch (final MalformedURLException ignored) {
    }

    if (originalURL == null) {
      adjustLinkResolutionCallback.resolvedLinkCallback(null);
      return;
    }

    if (!urlMatchesSuffix(originalURL.getHost(), resolveUrlSuffixArray)) {
      adjustLinkResolutionCallback.resolvedLinkCallback(
          AdjustLinkResolution.convertToUri(originalURL));
      return;
    }

    if (executor == null) {
      synchronized (expectedUrlHostSuffixArray) {
        if (executor == null) {
          executor = Executors.newSingleThreadExecutor();
        }
      }
    }

    final URL finalOriginalURL = originalURL;
    executor.execute(new Runnable() {
      @Override
      public void run() {
        requestAndResolve(finalOriginalURL, 0, adjustLinkResolutionCallback);
      }
    });
  }
  private static void resolveLink(final URL responseUrl, final URL previousUrl,
      final int recursionNumber, final AdjustLinkResolutionCallback adjustLinkResolutionCallback) {
    // return (possible null) previous url when the current one does not exist
    if (responseUrl == null) {
      adjustLinkResolutionCallback.resolvedLinkCallback(
          AdjustLinkResolution.convertToUri(previousUrl));
      return;
    }

    // return found url with expected host
    if (isTerminalUrl(responseUrl.getHost())) {
      adjustLinkResolutionCallback.resolvedLinkCallback(
          AdjustLinkResolution.convertToUri(responseUrl));
      return;
    }

    // return previous (non-null) url when it reached the max number of recursive tries
    if (recursionNumber > maxRecursions) {
      adjustLinkResolutionCallback.resolvedLinkCallback(
          AdjustLinkResolution.convertToUri(responseUrl));
      return;
    }

    requestAndResolve(responseUrl, recursionNumber, adjustLinkResolutionCallback);
  }

  private static void requestAndResolve(final URL urlToRequest, final int recursionNumber,
      final AdjustLinkResolutionCallback adjustLinkResolutionCallback) {
    final URL httpUrl = convertToHttp(urlToRequest);
    URL resolvedURL = null;
    HttpURLConnection ucon = null;
    try {
      ucon = (HttpURLConnection) httpUrl.openConnection();
      ucon.setInstanceFollowRedirects(false);

      ucon.connect();

      final String headerLocationField = ucon.getHeaderField("Location");
      if (headerLocationField != null) {
        resolvedURL = new URL(headerLocationField);
      }
    } catch (final Throwable ignored) {
    } finally {
      if (ucon != null) {
        ucon.disconnect();
      }

      resolveLink(resolvedURL, httpUrl, recursionNumber + 1, adjustLinkResolutionCallback);
    }
  }

  private static boolean isTerminalUrl(final String urlHost) {
    return urlMatchesSuffix(urlHost, expectedUrlHostSuffixArray);
  }

  private static boolean urlMatchesSuffix(final String urlHost, final String[] suffixArray) {
    if (urlHost == null) {
      return false;
    }

    if (suffixArray == null) {
      return false;
    }

    for (final String expectedUrlHostSuffix : suffixArray) {
      if (urlHost.endsWith(expectedUrlHostSuffix)) {
        return true;
      }
    }

    return false;
  }

  private static URL convertToHttp(final URL urlToConvert) {
    if (urlToConvert == null) {
      return urlToConvert;
    }

    final String stringUrlToConvert = urlToConvert.toExternalForm();

    if (stringUrlToConvert == null) {
      return urlToConvert;
    }

    if (!stringUrlToConvert.startsWith("http:")) {
      return urlToConvert;
    }

    URL convertedUrl = urlToConvert;
    try {
      convertedUrl = new URL("http:" + stringUrlToConvert.substring(5));
    } catch (final MalformedURLException ignored) {
    }

    return convertedUrl;
  }

  private static Uri convertToUri(URL url) {
    if (url == null) {
      return null;
    }

    return Uri.parse(url.toString());
  }
}
