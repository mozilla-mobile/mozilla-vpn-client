/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import org.gradle.api.Plugin
import org.gradle.api.Project
import org.gradle.api.file.CopySpec
import org.gradle.api.file.DuplicatesStrategy
import org.gradle.kotlin.dsl.provideDelegate
import org.json.JSONArray
import java.io.BufferedReader
import java.io.File
import java.io.InputStreamReader
import javax.inject.Inject

// This Plugin will help in finding the right paths for QT.
//
class QtConfig : Plugin<Project> {
    override fun apply(project: Project) {
        if (!project.hasProperty("mozillavpn_qt_android_dir")) {
            error(
                "mozillavpn_qt_android_dir is not set as gradle property \n " +
                    "Please add it to gradle.properties or set is as env"
            )
        }
        if (!project.hasProperty("mozillavpn_qt_host_dir")) {
            error(ERROR_MESSAGE_NO_QT_HOST)
        }
        project.extensions.create("QtConfiguration", QTConfigurationExtension::class.java, project)
    }
}

open class QTConfigurationExtension
@Inject constructor(
    private val project: Project
) {
    @Suppress("unused")
    val host: String by lazy {
        val dir = File(project.properties["mozillavpn_qt_host_dir"].toString())
        if (!dir.exists()) {
            error("$ERROR_MESSAGE_NO_QT_HOST \n Value: ${dir.absolutePath}")
        }
        dir.absolutePath.toString()
    }
    @Suppress("unused")
    val qtAndroidRootDir: String by lazy {
        File(anyAndroid).parentFile.absolutePath
    }

    @Suppress("unused")
    val arm64: String by lazy { getPath(QT_arm64) }
    @Suppress("unused")
    val armv7: String by lazy { getPath(QT_armv7) }
    val x86: String by lazy { getPath(QT_x84) }
    @Suppress("unused")
    val x64: String by lazy { getPath(QT_x64) }
    @Suppress("unused")
    val anyAndroid: String by lazy {
        listOf(x64, x86, arm64, armv7).first { it.isNotEmpty() }
    }

    @Suppress("unused")
    val abis: String by lazy {
        var out = listOf(x86, arm64, x64, armv7).filter { it.isNotEmpty() }.map {
            qt_archMap[File(it).name]
        }
        // Massage the string a bit so gradle takes it as a valid abi-filter value
        val b = out.toString().replace("[", "").replace("]", "").replace(" ", "")
        println(b)
        b
    }
    @Suppress("unused")
    val hasArm64: Boolean by lazy { arm64.isNotEmpty() }
    @Suppress("unused")
    val hasArmv7: Boolean by lazy { armv7.isNotEmpty() }
    @Suppress("unused")
    val hasX86: Boolean by lazy { x86.isNotEmpty() }
    @Suppress("unused")
    val hasX64: Boolean by lazy { x64.isNotEmpty() }

    @Suppress("unused")
    val bindingsFolder: String by lazy { "$anyAndroid/src/android/java/src/org/qtproject/qt/android/bindings/" }

    private fun getPath(QT_ABI: String): String {
        val androidDir = File(project.properties["mozillavpn_qt_android_dir"].toString())
        if (!androidDir.exists()) {
            error("Android Qt Directory not existing")
        }
        val maybeABI = androidDir.resolve(QT_ABI)
        if (!maybeABI.exists()) {
            return ""
        }
        return maybeABI.absolutePath
    }

    private fun importQtPlugin(name: String, abi: String, target: String): CopySpec {
        val pluginPath = File("$abi/plugins/$name")
        if (!pluginPath.exists()) {
            return project.copySpec()
        }
        return project.copySpec {
            this.into(target)
            this.from(pluginPath)
        }
    }
    fun importQtPlugin(name: String): CopySpec {
        return project.copySpec {
            this.with(importQtPlugin(name, x64, NDK_x64))
            this.with(importQtPlugin(name, arm64, NDK_arm64))
            this.with(importQtPlugin(name, x86, NDK_x86))
            this.with(importQtPlugin(name, armv7, NDK_armv7))
        }
    }

    fun copyQMLCompat(abi: String, target: String): CopySpec {
        val copyTask = project.copySpec()
        val path = getPath(abi) + "/qml/Qt5Compat/GraphicalEffects/private"
        val Qt5CompatFolder = File(path)
        if (!Qt5CompatFolder.exists()) {
            return copyTask
        }
        copyTask.from(path) {
            include("*.so")
        }
        copyTask.into(target)
        copyTask.duplicatesStrategy = DuplicatesStrategy.EXCLUDE
        return copyTask
    }
    fun copyQMLCompat(): CopySpec {
        return project.copySpec {
            this.with(copyQMLCompat(x64, NDK_x64))
            this.with(copyQMLCompat(arm64, NDK_arm64))
            this.with(copyQMLCompat(x86, NDK_x86))
            this.with(copyQMLCompat(armv7, NDK_armv7))
        }
    }

    // Generates Gradle Copy Tasks to for our QML imports for the given ABI
    fun copyQMLPlugin(abi: String, target: String): CopySpec {
        val imports = getQMLImports(abi)
        if (imports.isEmpty) {
            // No imports for that abi
            return project.copySpec()
        }
        val copyTask = project.copySpec()
        copyTask.into(target)

        for (i in 0 until imports.length()) {
            val import = imports.getJSONObject(i)
            if (import.has("path")) { // Qt has found a static qml dependency
                val path = import.getString("path")
                copyTask.from(path) {
                    include("*.so")
                }
            }
        }
        return copyTask
    }
    fun copyQMLPlugin(): CopySpec {
        return project.copySpec {
            this.with(copyQMLPlugin(x64, NDK_x64))
            this.with(copyQMLPlugin(arm64, NDK_arm64))
            this.with(copyQMLPlugin(x86, NDK_x86))
            this.with(copyQMLPlugin(armv7, NDK_armv7))
        }
    }

    // Runs QMLImportscanner from qt_host against the
    // qt_android_<abi> folder, to find out what we need to bundle for qml
    private fun getQMLImports(abi: String): JSONArray {
        if (abi.isEmpty()) {
            return JSONArray()
        }
        val importScanner = File("$host/libexec/qmlimportscanner")
        if (!importScanner.exists()) {
            error("Did not found qmlimportscanner in $host/libexec/qmlimportscanner")
        }
        val repo_path = project.rootProject.projectDir.parent
        val qmlPath = File("$repo_path/src/ui")
        val qtQMLPath = File("$abi/qml")
        if (!qmlPath.exists()) {
            return JSONArray()
        }
        print("Calling: $importScanner -rootPath $qmlPath -importPath $qtQMLPath")
        val process =
            Runtime.getRuntime().exec("$importScanner -rootPath $qmlPath -importPath $qtQMLPath")
        val processOutput = StringBuilder()
        BufferedReader(
            InputStreamReader(process.inputStream)
        ).use { processOutputReader ->
            var line: String? = processOutputReader.readLine()
            while (line != null) {
                processOutput.append(line + System.lineSeparator())
                line = processOutputReader.readLine()
            }
            process.waitFor()
        }
        val out = processOutput.toString()
        try {
            return JSONArray(out)
        } catch (e: Exception) {
            error("Got Output of: $out \n $e")
        }
    }

    companion object {
        const val NDK_arm64 = "arm64-v8a"
        const val NDK_armv7 = "armeabi-v7a"
        const val NDK_x86 = "x86"
        const val NDK_x64 = "x86_64"

        const val QT_arm64 = "android_arm64_v8a"
        const val QT_armv7 = "android_armv7"
        const val QT_x84 = "android_x86"
        const val QT_x64 = "android_x86_64"

        val qt_archMap = mapOf(
            QT_arm64 to NDK_arm64,
            QT_x64 to NDK_x64,
            QT_x84 to NDK_x86,
            QT_armv7 to NDK_armv7
        )
    }
}

const val ERROR_MESSAGE_NO_QT_HOST = "mozillavpn_qt_host_dir is not set or does not exist \n " +
    "Please add it:  \n" +
    " \t via env MOZILLAVPN_QT_HOST_DIR=<../> \n" +
    " \t or write mozillavpn_qt_host_dir=~/<some folder> to either \n" +
    " \t \t  ~/.gradle/gradle.properties \n " +
    " \t \t  project/android/gradle.properties"

const val ERROR_MESSAGE_NO_ANDROID_DIR = "mozillavpn_qt_android_dir is not set or does not exist \n " +
    "Please add it:  \n" +
    " \t via env MOZILLAVPN_QT_ANDROID_DIR=<../> \n" +
    " \t or write mozillavpn_qt_android_dir=~/<some folder> to either \n" +
    " \t \t  ~/.gradle/gradle.properties \n " +
    " \t \t  project/android/gradle.properties"
