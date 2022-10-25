/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import org.gradle.api.Plugin
import org.gradle.api.Project
import org.gradle.kotlin.dsl.provideDelegate
import java.io.File
import java.util.*
import javax.inject.Inject

class VPNVersionPlugin : Plugin<Project> {
    override fun apply(project: Project) {
        project.extensions.create("VPNVersion", VPNVersionExtension::class.java, project)
    }
}

open class VPNVersionExtension
@Inject constructor(
    private val project: Project
) {
    @Suppress("unused")
    // The version code is a TimeStamp where we drop the last 3 digits,
    // so we roughly get a new code every 20 min
    val versionCode: Int by lazy {
        var timestamp = Date()
        if (project.hasProperty("VPN_PIN_TIMESTAMP")) {
            timestamp = Date(project.properties["VPN_PIN_TIMESTAMP"].toString())
        }
        val timeString = timestamp.time.toString()
        val slice = timeString.dropLast(3)
        slice.toInt()
    }
    @Suppress("unused")
    val versionName: String by lazy {
        val cmake = File("${project.rootDir.absolutePath}/../CMakeLists.txt")
        if (!cmake.exists()) {
            error("Could not find the Root cmake under ${cmake.absolutePath}")
        }
        val lines = cmake.readLines()
        val project = lines.find {
            it.startsWith("project(")
        }.orEmpty()
        val tokens = project.split(" ")
        tokens[tokens.indexOf("VERSION") + 1]
    }
}
