package com.adjust.sdk.scheduler;

import com.adjust.sdk.AdjustFactory;

public class RunnableWrapper implements Runnable {
  private Runnable runnable;

  RunnableWrapper(Runnable runnable) {
    this.runnable = runnable;
  }

  @Override
  public void run() {
    try {
      runnable.run();
    } catch (Throwable t) {
      AdjustFactory.getLogger().error(
          "Runnable error [%s] of type [%s]", t.getMessage(), t.getClass().getCanonicalName());
    }
  }
}
