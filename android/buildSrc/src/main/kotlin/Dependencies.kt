/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const val Mozilla_ANDROID_COMPONENT_VERSION = "101.0.7"

object Dependencies {
    const val org_jetbrains_kotlin_kotlin_serialization = "org.jetbrains.kotlin:kotlin-serialization:1.4.30-M1"
    const val org_jetbrains_kotlin_kotlin_gradle_plugin = "org.jetbrains.kotlin:kotlin-gradle-plugin:1.6.10"
    const val com_android_tools_build_gradle = "com.android.tools.build:gradle:4.0.0"
    const val androidx_core = "androidx.core:core-ktx:1.6.0"
    const val android_installreferrer = "com.android.installreferrer:installreferrer:2.2"
    const val android_billingclient = "com.android.billingclient:billing-ktx:4.0.0"
    const val androidx_lifecycle = "androidx.lifecycle:lifecycle-livedata-ktx:2.4.0-alpha02"
    const val androidx_security_security_crypto = "androidx.security:security-crypto:1.1.0-alpha03"
    const val androidx_security_security_identity_credential = "androidx.security:security-identity-credential:1.0.0-alpha02"
    const val com_google_android_gms_play_services_ads_identifier = "com.google.android.gms:play-services-ads-identifier:17.0.1"
    const val org_jetbrains_kotlinx_kotlinx_serialization_json = "org.jetbrains.kotlinx:kotlinx-serialization-json:1.2.2"
    const val com_android_tools_desugar_jdk_libs = "com.android.tools:desugar_jdk_libs:1.0.10"
    const val org_mozilla_components_tooling_glean_gradle = "org.mozilla.components:tooling-glean-gradle:$Mozilla_ANDROID_COMPONENT_VERSION"
    const val org_mozilla_telemetry_glean = "org.mozilla.telemetry:glean:44.2.0"
}
