/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import org.gradle.kotlin.dsl.`kotlin-dsl`

plugins {
    // Required workaround for: https://github.com/gradle/gradle/issues/16345
    `kotlin-dsl`.version("2.3.3")
}
repositories {
    mavenCentral()
}
