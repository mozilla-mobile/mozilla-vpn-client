import org.gradle.api.Plugin
import org.gradle.api.Project
import org.gradle.api.file.CopySpec
import org.gradle.internal.impldep.com.google.gson.JsonArray
import org.gradle.kotlin.dsl.provideDelegate
import org.json.JSONArray
import org.json.JSONObject
import java.io.File
import javax.inject.Inject
import java.io.BufferedReader
import java.io.InputStreamReader


// This Plugin will help in finding the right paths for QT.
//
class QtConfig : Plugin<Project> {
    private val qtAndroidDir ="~/Qt/6.2.4" // Just a dummy, will be overwritten hopefully

    override fun apply(project: Project) {
        if(!project.hasProperty("mozillavpn_qt_android_dir")){
            error("mozillavpn_qt_android_dir is not set as gradle property \n "+
                    "Please add it to gradle.properties or set is as env"
            )
        }
        if(!project.hasProperty("mozillavpn_qt_host_dir")){
            error("mozillavpn_qt_host_dir is not set as gradle property \n "+
                    "Please add it to gradle.properties or set is as env"
            )
        }
        project.extensions.create("QtConfiguration", QTConfigurationExtension::class.java, project)
    }
}


open class QTConfigurationExtension
@Inject constructor(
    private val project: Project
){
    @Suppress("unused")
    val host :String by lazy {
        val dir = File(project.properties["mozillavpn_qt_host_dir"].toString())
        if(!dir.exists()){
            error("You need to define a QT host dir")
        }
        dir.absolutePath.toString()
    }
    @Suppress("unused")
    val qtAndroidRootDir :String by lazy{
        File(anyAndroid).parentFile.absolutePath
    }


    @Suppress("unused")
    val arm64:String by lazy { getPath(QT_arm64) }
    @Suppress("unused")
    val armv7:String by lazy { getPath(QT_armv7) }
    val x86:String by lazy { getPath(QT_x84) }
    @Suppress("unused")
    val x64:String by lazy { getPath(QT_x64) }
    @Suppress("unused")
    val anyAndroid :String by lazy{
        listOf(x64,x86,arm64,armv7).first { it.isNotEmpty() }
    }


    @Suppress("unused")
    val abis: String by lazy {
        var out = listOf(x86,arm64,x64,armv7).filter { it.isNotEmpty() }.map {
            qt_archMap[File(it).name];
        }
        // Massage the string a bit so gradle takes it as a valid abi-filter value
       val b = out.toString().replace("[","").replace("]","").replace(" ", "")
        println(b)
        b
    }
    @Suppress("unused")
    val hasArm64 :Boolean  by lazy { arm64.isNotEmpty() }
    @Suppress("unused")
    val hasArmv7 :Boolean  by lazy { armv7.isNotEmpty() }
    @Suppress("unused")
    val hasX86 : Boolean  by lazy { x86.isNotEmpty() }
    @Suppress("unused")
    val hasX64 : Boolean  by lazy { x64.isNotEmpty() }

    @Suppress("unused")
    val bindingsFolder : String by lazy { "$anyAndroid/src/android/java/src/org/qtproject/qt/android/bindings/" }


    private fun getPath(QT_ABI:String) : String{
        val androidDir = File(project.properties["mozillavpn_qt_android_dir"].toString())
        if(!androidDir.exists()){
            error("Android Qt Directory not existing")
        }
        val maybeABI = androidDir.resolve(QT_ABI);
        if(!maybeABI.exists()){
            return ""
        }
        return maybeABI.absolutePath
    }


    // Generates Gradle Copy Tasks for the given Android-Plattform-Plugin for a given ABI
    fun copyAndroidPlugin(abi:String, target:String):CopySpec{
        val pluginPath = File("$abi/plugins/platforms")
        if(!pluginPath.exists()){
            return project.copySpec()
        }
        return project.copySpec {
            this.into(target)

            this.from(pluginPath)
        }
    }

    // Generates Gradle Copy Tasks to for our QML imports for the given ABI
    fun copyQMLPlugin(abi:String, target:String):CopySpec{
        val imports = getQMLImports(abi)
        if(imports.isEmpty){
            // No imports for that abi
            return project.copySpec()
        }
        val copyTask = project.copySpec()
        copyTask.into(target)

        for(i in 0 until imports.length()){
            val import = imports.getJSONObject(i);
            if(import.has("path")){ // Qt has found a static qml dependency
                val path = import.getString("path")
                copyTask.from(path) {
                    include("*.so")
                }
            }
        }
        return copyTask
    }

    // Runs QMLImportscanner from qt_host against the
    // qt_android_<abi> folder, to find out what we need to bundle for qml
    private fun getQMLImports(abi: String): JSONArray {
        val importScanner = File("$host/libexec/qmlimportscanner")
        if (!importScanner.exists()) {
            error("Did not found qmlimportscanner in $host/libexec/qmlimportscanner")
        }
        val repo_path = project.rootProject.projectDir.parent
        val qmlPath = File("$repo_path/src/ui")
        val qtQMLPath = File("$abi/qml")
        if(!qmlPath.exists()){
            return JSONArray()
        }
        val process =
            Runtime.getRuntime().exec("$importScanner -rootPath $qmlPath -importPath $qtQMLPath");
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
        return JSONArray(processOutput.toString())
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



