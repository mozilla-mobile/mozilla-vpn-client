package org.mozilla.firefox.vpn.daemon

import org.json.JSONObject
import org.junit.Assert.assertEquals
import org.junit.Assert.assertNotNull
import org.junit.Assert.assertNull
import org.junit.Test

class NotificationUtilTest {

    @Test
    fun cannedNotificationParsing() {
        val test = JSONObject(
            """{'messages':{ 
                'connectedHeader':'header',
                'connectedBody': 'body',
                'disconnectedHeader': 'header',
                'disconnectedBody': 'body',
                'productName' :'moztest',
                'requestedScreen': 'aaaaa'
             }}
        """,
        )

        val res = CannedNotification(test)
        assertNotNull(res)
        if (res == null) {
            return
        }
        assertEquals(res.connectedMessage.header, "header")
        assertEquals(res.connectedMessage.body, "body")

        assertEquals(res.disconnectedMessage.header, "header")
        assertEquals(res.disconnectedMessage.body, "body")
        assertEquals(res.productName, "moztest")
    }

    /**
     * It should not throw if we do not provide a screen url for the notification
     */
    @Test
    fun cannedNotificationParsingWithoutScreenURL() {
        val test = JSONObject(
            """{'messages':{ 
                'connectedHeader':'header',
                'connectedBody': 'body',
                'disconnectedHeader': 'header',
                'disconnectedBody': 'body',
                'productName' :'moztest',
             }}
        """,
        )

        val res = CannedNotification(test)
        assertNotNull(res)
        if (res == null) {
            return
        }
        assertEquals(res.connectedMessage.header, "header")
        assertEquals(res.connectedMessage.body, "body")

        assertEquals(res.disconnectedMessage.header, "header")
        assertEquals(res.disconnectedMessage.body, "body")
        assertEquals(res.productName, "moztest")

        assertNull(res.requestedScreen)
    }

    @Test
    fun cannedNotificationParsingReturnsNullOnMalformat() {
        val test = JSONObject(
            """{'mèsságes':{ 
                'connectedHeader':'header',
                'connectedBody': 'body',
                'disconnectedHeader': 'header',
                'disconnectedBody': 'body',
                'productName' :'moztest',
             }}
        """,
        )

        val res = CannedNotification(test)
        assertNull(res)
    }
}
