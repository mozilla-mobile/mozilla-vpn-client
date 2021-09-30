package com.adjust.sdk.scheduler;

import com.adjust.sdk.AdjustFactory;

import java.util.concurrent.Callable;
import java.util.concurrent.RejectedExecutionHandler;
import java.util.concurrent.ScheduledFuture;
import java.util.concurrent.ScheduledThreadPoolExecutor;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;

public class SingleThreadFutureScheduler implements FutureScheduler {
  private ScheduledThreadPoolExecutor scheduledThreadPoolExecutor;

  public SingleThreadFutureScheduler(final String source, boolean doKeepAlive) {
    this.scheduledThreadPoolExecutor =
        new ScheduledThreadPoolExecutor(1, new ThreadFactoryWrapper(source),
            new RejectedExecutionHandler() { // Logs rejected runnables rejected from the entering
                                             // the pool
              @Override
              public void rejectedExecution(Runnable runnable, ThreadPoolExecutor executor) {
                AdjustFactory.getLogger().warn(
                    "Runnable [%s] rejected from [%s] ", runnable.toString(), source);
              }
            });

    if (!doKeepAlive) {
      scheduledThreadPoolExecutor.setKeepAliveTime(10L, TimeUnit.MILLISECONDS);
      scheduledThreadPoolExecutor.allowCoreThreadTimeOut(true);
    }
  }

  @Override
  public ScheduledFuture<?> scheduleFuture(Runnable command, long millisecondDelay) {
    return scheduledThreadPoolExecutor.schedule(
        new RunnableWrapper(command), millisecondDelay, TimeUnit.MILLISECONDS);
  }

  @Override
  public <V> ScheduledFuture<V> scheduleFutureWithReturn(
      final Callable<V> callable, long millisecondDelay) {
    return scheduledThreadPoolExecutor.schedule(new Callable<V>() {
      @Override
      public V call() {
        try {
          return callable.call();
        } catch (Throwable t) {
          AdjustFactory.getLogger().error(
              "Callable error [%s] of type [%s]", t.getMessage(), t.getClass().getCanonicalName());
          return null;
        }
      }
    }, millisecondDelay, TimeUnit.MILLISECONDS);
  }

  @Override
  public ScheduledFuture<?> scheduleFutureWithFixedDelay(
      Runnable command, long initialMillisecondDelay, long millisecondDelay) {
    return scheduledThreadPoolExecutor.scheduleWithFixedDelay(new RunnableWrapper(command),
        initialMillisecondDelay, millisecondDelay, TimeUnit.MILLISECONDS);
  }

  @Override
  public void teardown() {
    scheduledThreadPoolExecutor.shutdownNow();
  }
}
