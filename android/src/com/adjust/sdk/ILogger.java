package com.adjust.sdk;

public interface ILogger {
  void setLogLevel(LogLevel logLevel, boolean isProductionEnvironment);

  void setLogLevelString(String logLevelString, boolean isProductionEnvironment);

  void verbose(String message, Object... parameters);

  void debug(String message, Object... parameters);

  void info(String message, Object... parameters);

  void warn(String message, Object... parameters);
  void warnInProduction(String message, Object... parameters);

  void error(String message, Object... parameters);

  void Assert(String message, Object... parameters);

  void lockLogLevel();
}
