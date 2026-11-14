# TEST_REPORT.md

## Sessão 2 — Fase 2 (fundação Android), testes executados

Ambiente: sandbox Linux x86-64, g++ com C++20. Sem Android SDK/NDK, sem aparelho.

### Testes do verificador de ROM (host) — 18 verificações, todas passaram

Comando: ver `BUILD_INSTRUCTIONS.md` §1. Compilado com `-Wall -Wextra -Werror`.

| Verificação | Resultado |
|---|---|
| SHA-1 de `abc`, de vazio e do vetor de 56 bytes (vetores FIPS 180) | ok |
| ROM real tem 32 MiB | ok |
| ROM real: status Ok, formato z64, versão "NTSC 1.2 (US)" | ok |
| ROM real: SHA-1 `41b3bdc48d98c48529219919015a1af22f5057c2`, CRC1 `0x693BA2AE`, CZLE rev 2 | ok |
| Mesma ROM convertida em memória para v64: mesmo SHA-1 | ok |
| Mesma ROM convertida em memória para n64: mesmo SHA-1 | ok |
| 1 bit alterado: `UnknownHash` | ok |
| Título alterado e hash desconhecido: `NotOcarina` | ok |
| Arquivo truncado: `TooSmallOrBadSize` | ok |
| 32 MiB de zeros: `UnknownFormat` | ok |
| Arquivo vazio: `TooSmallOrBadSize` | ok |
| Cabeçalho de .zip: `IsZipArchive` | ok |

### Verificações de sintaxe
| Item | Resultado |
|---|---|
| `jni_bridge.cpp` compila no host contra o `jni.h` do JDK 21 (`-fsyntax-only`, `-Werror`) | ok |
| Workflow `android.yml` é YAML válido, com os 3 jobs esperados | ok |

### NÃO testado (não confundir com "passou")
| Item | Motivo |
|---|---|
| Build Gradle/NDK do APK | Ambiente sem acesso ao Google Maven, Gradle e NDK. Primeiro build real será no GitHub Actions ou no seu PC |
| Código Kotlin (`MainActivity`, `HardwareProbe`, `NativeBridge`) | Nunca compilado. Sem `kotlinc` aqui |
| Carregamento da lib nativa no Android (`System.loadLibrary`) | Depende do APK |
| Sonda de GLES/Vulkan/RAM no Realme Note 50 | Depende do APK e do aparelho |
| Verificação de ROM via seletor de arquivos (SAF + `detachFd`) | Depende do APK e do aparelho |
| Job `lus-arm64-probe` | Só roda no GitHub Actions |

### Desempenho
Sem medições de FPS, RAM, temperatura ou estabilidade. O jogo ainda não roda.
