package org.mozilla.guardian.tunnel

class WireGuardGo {
    companion object {
        init {
            System.loadLibrary("tunnel") // Matches `libwg-go.so`
        }
        @JvmStatic
        external fun wgTurnOn(interfaceName: String, tunFd: Int, settings: String): Int
        @JvmStatic
        external fun wgTurnOff(handle: Int)
        @JvmStatic
        external fun wgGetSocketV4(handle: Int): Int
        @JvmStatic
        external fun wgGetSocketV6(handle: Int): Int
        @JvmStatic
        external fun wgGetConfig(handle: Int): String?
        @JvmStatic
        external fun wgVersion(): String?
        @JvmStatic
        external fun wgGetLatestHandle(): Int
    }

}