plugins {
    id("com.android.application")
    id("org.jetbrains.kotlin.android")
}

android {
    namespace = "com.ocarinamobile.legacy"
    compileSdk = 34
    ndkVersion = "26.1.10909125"

    defaultConfig {
        applicationId = "com.ocarinamobile.legacy"
        minSdk = 26
        targetSdk = 34
        versionCode = 1
        versionName = "0.1.0-fase2"

        // Só arm64, como o pedido manda.
        ndk { abiFilters += listOf("arm64-v8a") }

        externalNativeBuild {
            cmake {
                arguments += listOf("-DANDROID_STL=c++_static")
                cppFlags += listOf("-std=c++20")
            }
        }
    }

    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
            version = "3.22.1"
        }
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            // Assinatura de debug apenas nesta fase, para instalar sem chave própria.
            signingConfig = signingConfigs.getByName("debug")
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }
    kotlinOptions { jvmTarget = "17" }
}

dependencies {
    // Sem bibliotecas externas nesta fase: só o SDK do Android.
}
