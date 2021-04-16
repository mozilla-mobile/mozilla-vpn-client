#ifndef ANDROIDVPNACTIVITY_H
#define ANDROIDVPNACTIVITY_H

/* This class defines the Natives found in
 * android/../Qt/VpnActivity.java
 */

class AndroidVPNActivity {
 public:
  static void init();
  static bool handleBackButton();

 private:
  AndroidVPNActivity();
};

#endif  // ANDROIDVPNACTIVITY_H
