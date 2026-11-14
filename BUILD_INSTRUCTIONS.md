# BUILD_INSTRUCTIONS.md

Estado: **Fase 2, parcial.** Os comandos abaixo marcados "testado" foram executados no ambiente de desenvolvimento. Os marcados "não testado" nunca rodaram, porque esse ambiente não alcança o Google Maven, o Gradle nem o repositório do NDK.

## 1. Testes do verificador de ROM no host (testado)

Requer g++ com C++20.

```
g++ -std=c++20 -Wall -Wextra -Werror -O2 -o rom_verifier_test \
  tests/rom_verifier_test.cpp native/rom_verifier/rom_verifier.cpp native/rom_verifier/sha1.cpp
./rom_verifier_test "/caminho/para/sua/rom.z64"
```

Sem argumento, roda só os testes sintéticos (SHA-1 com vetores padrão). Com a ROM, roda também a verificação real. A ROM é só lida.

## 2. APK pelo GitHub Actions (não testado — recomendado)

1. Crie um repositório no GitHub com o conteúdo desta pasta. O `.gitignore` já bloqueia `.z64`, `.v64`, `.n64`, `.o2r`, `.otr` e `.zip`. **Confira que nenhuma ROM entrou antes do primeiro push.**
2. O workflow `.github/workflows/android.yml` roda sozinho a cada push, com três jobs:
   - `host-tests`: testes sintéticos do verificador.
   - `apk`: instala SDK 34, build-tools 34.0.0, NDK 26.1.10909125 e CMake 3.22.1, e roda `./gradlew assembleDebug assembleRelease`. O APK sai como artefato `apk-fase2`.
   - `lus-arm64-probe`: experimento. Tenta compilar o libultraship (commit `da6daf1a`) para Android arm64 com os comandos do README dele. Os logs saem como artefato `lus-arm64-logs`, com sucesso ou falha. Não compila o jogo.
3. Baixe o APK pela aba Actions → execução → Artifacts.

## 3. APK no seu PC (não testado)

Requer Android Studio (ou SDK + JDK 17), NDK `26.1.10909125` e CMake `3.22.1` pelo SDK Manager.

```
cd android
./gradlew assembleDebug
# APK em android/app/build/outputs/apk/debug/
```

Versões do Android Gradle Plugin (8.7.3), Kotlin (2.0.21) e Gradle (8.14.3) são escolhas minhas e **nunca foram resolvidas de verdade**. Se o build reclamar de versão, o erro é o dado que preciso.

## 4. Referência: build do libultraship sozinho (do README dele)

```
cmake -S . -Bbuild-android -GNinja \
  -DCMAKE_SYSTEM_NAME=Android \
  -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake \
  -DANDROID_NDK=$ANDROID_NDK_HOME \
  -DANDROID_ABI=arm64-v8a \
  -DANDROID_PLATFORM=latest \
  -DANDROID_STL=c++_static \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build-android
```
