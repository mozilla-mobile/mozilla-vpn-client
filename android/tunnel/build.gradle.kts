// tunnel/build.gradle.kts

import org.gradle.api.Project
import java.io.FileInputStream
import java.util.Properties

plugins {
    id("com.android.library")
    id("kotlin-android")
}

// --- Go Configuration Helper (Simplified) ---
// Reads local.properties JUST to find the go executable directory if specified
fun getGoEnvArgsForNdkBuild(project: Project): Map<String, String> {
    val args = mutableMapOf<String, String>()
    val localProperties = Properties()
    val localPropertiesFile = project.rootProject.file("local.properties")
    if (localPropertiesFile.isFile) {
        try {
            FileInputStream(localPropertiesFile).use { localProperties.load(it) }
        } catch (
            e: Exception,
        ) {
            project.logger.warn("Could not read local.properties: ${e.message}")
        }
    }

    // Only need the Go executable path to pass to Make
    val goPathProp = localProperties.getProperty("go.path") // Expect directory containing 'go'

    // Determine Go executable path (respecting property > PATH > default 'go')
    val goExeName = "go" + if (System.getProperty("os.name").toLowerCase().contains("windows")) ".exe" else ""
    val goExePath =
        if (!goPathProp.isNullOrBlank()) {
            project
                .file(goPathProp)
                .resolve(goExeName)
                .takeIf { it.canExecute() }
                ?.absolutePath
        } else {
            // Check PATH - Note: This check isn't perfect from Gradle config phase
            // Rely on Make to find 'go' on PATH if property isn't set/valid
            null
        } ?: "go" // Default to 'go' if property invalid or not found on PATH easily

    args["GO_EXE"] = goExePath // Pass the resolved path or just 'go'

    // GOPATH/GOCACHE can be handled within Makefile defaults or relative paths
    project.logger.lifecycle("Passing GO_EXE='${args["GO_EXE"]}' to ndk-build.")
    return args
}

android {
    compileSdk = Config.compileSdkVersion
    ndkVersion = Config.ndkVersion

    namespace = "org.mozilla.firefox.vpn.tunnel"

    defaultConfig {
        minSdk = 24
        targetSdk = Config.targetSdkVersion

        externalNativeBuild {
            cmake {
                targets.add("go_shared_lib")
            }
        }
    }
    // Configure ndk-build integration
    externalNativeBuild {
        cmake {
            path("src/go/CMakeLists.txt")
        }
    }

    buildFeatures {
        aidl = false
        buildConfig = false
    }

    // Ensure .so files in src/main/jniLibs are packaged in the AAR
    sourceSets.getByName("main").jniLibs.srcDirs("src/main/jniLibs")

    // AGP typically finds .so files from ndk-build output (libs/<abi>) automatically.
    // Explicitly setting sourceSets.main.jniLibs might interfere or be redundant.
    // Let's rely on the default behavior first.
    // sourceSets.getByName("main").jniLibs.srcDirs("libs") // Default ndk-build output dir

    buildTypes {
        release {
            isMinifyEnabled = false
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_1_8
        targetCompatibility = JavaVersion.VERSION_1_8
    }
    kotlinOptions {
        jvmTarget = "1.8"
    }
}

dependencies {
    implementation("androidx.core:core-ktx:1.12.0")
}
