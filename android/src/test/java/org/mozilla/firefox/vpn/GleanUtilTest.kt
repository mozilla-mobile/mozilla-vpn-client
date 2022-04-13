package org.mozilla.firefox.vpn

import mozilla.telemetry.glean.private.EventMetricType
import mozilla.telemetry.glean.private.NoExtraKeys
import mozilla.telemetry.glean.private.NoExtras
import org.junit.Assert.assertEquals
import org.junit.Assert.assertNotNull
import org.junit.Test
import org.mozilla.firefox.vpn.qt.GleanUtil
import org.mozilla.firefox.vpn.glean.GleanEvent

/**
 * Example local unit test, which will execute on the development machine (host).
 *
 * See [testing documentation](http://d.android.com/tools/testing).
 */
class GleanUtilTest {
    @Test
    fun addition_isCorrect() {
        assertEquals(4, 2 + 2)
    }


    @Test
    fun check_getPropertyNoExtras(){
        val y= GleanUtil.getSample("authenticationFailure")
        assertNotNull(y)
    }
}
