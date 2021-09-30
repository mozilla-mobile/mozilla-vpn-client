package com.adjust.sdk.scheduler;

import java.util.concurrent.Callable;
import java.util.concurrent.Future;

/**
 * Created by nonelse on 12.09.17.
 */

public interface ThreadExecutor {
  void submit(Runnable task);
  void teardown();
}
