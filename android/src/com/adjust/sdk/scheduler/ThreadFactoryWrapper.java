package com.adjust.sdk.scheduler;

import android.os.Process;

import com.adjust.sdk.AdjustFactory;
import com.adjust.sdk.Constants;

import java.util.concurrent.Executors;
import java.util.concurrent.ThreadFactory;

public class ThreadFactoryWrapper implements ThreadFactory {
  private String source;

  public ThreadFactoryWrapper(String source) {
    this.source = source;
  }

  @Override
  public Thread newThread(Runnable runnable) {
    Thread thread = Executors.defaultThreadFactory().newThread(runnable);

    thread.setPriority(Process.THREAD_PRIORITY_BACKGROUND + Process.THREAD_PRIORITY_MORE_FAVORABLE);
    thread.setName(Constants.THREAD_PREFIX + thread.getName() + "-" + source);
    thread.setDaemon(true);

    thread.setUncaughtExceptionHandler(new Thread.UncaughtExceptionHandler() {
      @Override
      public void uncaughtException(Thread th, Throwable tr) {
        AdjustFactory.getLogger().error(
            "Thread [%s] with error [%s]", th.getName(), tr.getMessage());
      }
    });

    return thread;
  }
}
