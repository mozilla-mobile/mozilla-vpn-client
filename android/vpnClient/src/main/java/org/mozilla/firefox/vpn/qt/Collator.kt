/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.firefox.vpn.qt

import android.icu.text.Collator
import android.icu.util.ULocale

object MZCollator {
    @JvmStatic
    fun compareStrings(a: String, b: String, locale: String): Int {
        val collator = Collator.getInstance(ULocale(locale))
        return collator.compare(a, b).coerceIn(-1, 1)
    }
}
