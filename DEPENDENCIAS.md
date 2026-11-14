# DEPENDENCIAS.md — Fase 1

Dependências da base escolhida (Shipwright + libultraship + Torch) e o que muda no Android. Versões são as declaradas nos arquivos CMake lidos; "pin" = versão fixada pelo próprio projeto.

Legenda de status Android:
- **Pronto**: o CMake do LUS já trata Android.
- **A verificar**: existe indício, falta compilar.
- **Trabalho novo**: precisa de código ou substituição.

## 1. Em tempo de execução (dentro do APK)

| Dependência | Uso | Versão (pin) | Licença | Android |
|---|---|---|---|---|
| libultraship (LUS) | Camada N64 → PC: vídeo, áudio, entrada, recursos | `da6daf1a` | MIT | Pronto (README, `android.cmake`, `USE_OPENGLES`) |
| Fast3D (dentro do LUS) | Interpreta display lists do RDP | — | MIT | GLES3 suportado; ver riscos de shader em `ARQUITETURA.md` |
| SDL2 | Janela, contexto GL, entrada, áudio | 2.32.10 (baixado se ausente) | zlib | Pronto. Exige `SDLActivity` Java/Kotlin |
| Dear ImGui | Menus do jogo | via LUS | MIT | Pronto, com escala 2× em Android; UI desktop, precisa de tratamento para toque |
| nlohmann-json | Configuração | 3.12.0 | MIT | Pronto |
| tinyxml2 | XML | 11.0.0 (LUS); 10.0.0 (Torch) | zlib | Pronto |
| spdlog | Log | 1.16.0 | MIT | Pronto |
| libzip | Arquivos `.o2r` | 1.11.4 | BSD-3 | Pronto |
| StormLib | Arquivos `.mpq`/`.otr` | via LUS | MIT | A verificar |
| zlib, bzip2 | Compressão | via StormLib/libzip | zlib / bzip2 | Pronto (NDK fornece zlib) |
| SDL2_net | Rede do SoH (modos online) | `find_package` | zlib | A verificar. Candidata a **remoção**, pois o app é offline |
| Ogg, Vorbis, Opus, OpusFile | Música customizada | `find_package(... CONFIG REQUIRED)` | BSD | A verificar. Candidatas a **remoção/opcional** se não houver música customizada |
| libc++ | STL | `c++_static` | Apache-2.0 com exceção LLVM | Pronto |

Observação: o CMake do SoH exige Ogg/Vorbis/Opus/OpusFile no ramo que li. Se dá para tornar opcional sem quebrar o jogo é uma **pergunta aberta** para a Fase 2.

## 2. Em tempo de extração (primeira execução, ROM → `.o2r`)

| Dependência | Uso | Licença | Android |
|---|---|---|---|
| Torch | Converte a ROM em `.o2r` | MIT | CMake tem ramo `ANDROID` (zlib do sistema). **A verificar** se compila e roda |
| yaml-cpp | Manifestos de assets do Torch | MIT | A verificar |
| libgfxd | Desmontagem de display lists | MIT | A verificar |
| dr_libs | Decodificação de áudio | Domínio público / MIT-0 | A verificar |
| Microsoft GSL | Utilitários C++ | MIT | A verificar |
| `n64graphics`, `binarytools` (em `lib/` do Torch) | Imagens e I/O binário | — | A verificar |

Alternativas se o Torch não rodar no aparelho (decisão só depois de investigar a causa, como o pedido manda):
1. Extrair no PC do usuário e importar o `.o2r` pronto no app. Pior experiência, mas simples.
2. Usar a extração antiga (`ZAPDTR` + `OTRExporter`), como o fork Android faz. Mais código para manter.

## 3. Em tempo de build

| Ferramenta | Uso | Estado nesta sessão |
|---|---|---|
| Android NDK | Compilar C/C++ para arm64-v8a | **Ausente** (`ANDROID_NDK_HOME` vazio) |
| Android SDK + Gradle | Empacotar o APK | SDK ausente; Gradle 8.14.3 presente |
| CMake, Ninja | Build nativo | Presentes (CMake 3.28.3). O fork Android pede CMake 3.31.5 no Gradle; a versão mínima real do SoH não foi verificada |
| JDK | Gradle | Presente |
| Python 3 | Scripts do Shipwright | Presente |
| Referência de toolchain (fork) | NDK `26.0.10792818`, minSdk 24, targetSdk 33 | Só referência; a versão a usar será decidida na Fase 2 |

## 4. Dependências que o pedido listou e a decisão proposta

| Item do pedido | Decisão proposta | Razão |
|---|---|---|
| C++20 | Manter para o código **novo** (shell, verificador, salvamento). O código do jogo (C + C++ do SoH) continua com o padrão que o projeto usa | Não vou migrar código do SoH só para C++20 |
| Android NDK, CMake, JNI | Manter | Alinhado ao LUS |
| Kotlin | UI do app: tela inicial, seletor de ROM, configurações, layout de toque | — |
| OpenGL ES 3.0 | Manter | É o caminho do LUS |
| Vulkan opcional | **Adiar**. O LUS não tem backend Vulkan e o Fast3D teria de ganhar um | Custo alto, sem evidência de ganho |
| Oboe/AAudio | **Começar com o áudio SDL2** que o LUS já usa. Avaliar Oboe só se o profiling mostrar latência ou falhas de áudio | Filosofia do pedido: "faça profiling antes de otimizar" |
| Armazenamento privado | Manter | Ver `ARQUITETURA.md` §6 |

## 4.1 Dependências que o app novo adiciona

| Componente | Função |
|---|---|
| Kotlin + AndroidX | UI nativa |
| `SDLActivity` (Java do SDL2) | Ponte obrigatória entre Android e SDL2. Uma `MainActivity` em Kotlin herda dela |
| Storage Access Framework | Escolher a ROM sem permissão ampla de arquivos |
| (opcional, futuro) Oboe | Só se o profiling pedir |

## 5. Licenças — visão consolidada

| Componente | Licença |
|---|---|
| libultraship, Fast3D, Torch, Dear ImGui, nlohmann-json, spdlog, StormLib | MIT |
| SDL2, zlib, tinyxml2 | zlib |
| libzip | BSD-3 |
| **Shipwright (soh/)** | **Sem arquivo de licença na raiz** (há um `LICENSE.md` só em `3drando`) |
| **zeldaret/oot** | **Sem arquivo de licença na raiz** |
| Zelda64Recomp (não usado) | GPLv3 |

Regra do projeto, vinda do pedido: nenhuma ROM, música, modelo ou textura protegida entra no repositório ou no APK. O `.o2r` gerado no aparelho contém dados derivados do jogo e **fica no armazenamento privado do app**, nunca no repositório.
