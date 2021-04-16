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
