/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.firefox.vpn.qt;

import android.app.Activity;
import android.util.Log;

import com.google.android.play.core.review.ReviewInfo;
import com.google.android.play.core.review.ReviewManager;
import com.google.android.play.core.review.ReviewManagerFactory;
import com.google.android.play.core.tasks.Task;

public class VPNAppReview {
  private static final String TAG = "VPNAppReview";

  public static void appReviewRequested(Activity activity) {
    Log.d(TAG, "App review requested");
    ReviewManager manager = ReviewManagerFactory.create(activity);
    Task<ReviewInfo> request = manager.requestReviewFlow();
    request.addOnCompleteListener(task -> {
      Log.d(TAG, "ReviewInfo object received");
      if (!task.isSuccessful()) {
        Log.d(TAG, "Something wrong happened");
        return;
      }

      ReviewInfo reviewInfo = task.getResult();
      Task<Void> flow = manager.launchReviewFlow(activity, reviewInfo);
      flow.addOnCompleteListener(finalTask -> {
        // The flow has finished. The API does not indicate whether the user
        // reviewed or not, or even whether the review dialog was shown. Thus, no
        // matter the result, we continue our app flow.
        Log.d(TAG, "The review flow is completed");
      });
    });
  }
}
