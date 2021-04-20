/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ANDROIDVPNACTIVITY_H
#define ANDROIDVPNACTIVITY_H

/* This class defines the Natives found in
 * android/../Qt/VpnActivity.java
 */
#include "jni.h"
class AndroidVPNActivity {
 public:
  static void init();
  static bool handleBackButton(JNIEnv* env, jobject thiz);

 private:
  AndroidVPNActivity();
};

#endif  // ANDROIDVPNACTIVITY_H
