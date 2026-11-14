# AUDITORIA.md — Fase 1

Projeto: **Ocarina Mobile Legacy**
Data da auditoria: 13/11/2026
Escopo: leitura de código e documentação de três repositórios, mais verificação da ROM fornecida. **Nada foi compilado e nada foi testado em aparelho nesta fase.** Todo número de desempenho ou memória abaixo é estimativa e está marcado como tal.

Legenda de confiança usada neste documento:
- **[VERIFICADO]** li no código ou na documentação do repositório, ou calculei aqui.
- **[FONTE EXTERNA]** vem de página web, citada em "Fontes".
- **[ESTIMATIVA]** raciocínio meu, ainda sem medição.
- **[NÃO VERIFICADO]** não consegui checar.

---

## 1. Resumo executivo

| Pergunta | Resposta |
|---|---|
| Melhor base técnica | **Shipwright (HarbourMasters) + libultraship**, com um shell Android novo. |
| Por que não o `zeldaret/oot` sozinho | É uma decompilação, não um port. Não tem camada de plataforma (vídeo, áudio, entrada, libultra). Construir essa camada é reescrever o que o libultraship já faz. |
| Por que não o Zelda64Recomp | Hoje roda só **Majora's Mask**. O suporte a Ocarina of Time aparece como "em breve" e **não há código de OoT** no snapshot que li. O renderizador (RT64) exige Vulkan 1.2 / D3D12 / Metal no nível de PC. |
| ROM fornecida | **NTSC-U v1.2**, formato z64. Está na lista de versões suportadas pelo Shipwright (SHA-1 conferido). |
| Já existe port Android do Shipwright? | Sim, não oficial (fork de Waterdish). Serve como referência, não como base (ver §5). |
| Maior risco | Desempenho no Mali-G57 MP1 com 4 GB de RAM. Não há dado medido. O primeiro protótipo existe para medir isso cedo. |
| Segundo maior risco | Licença do código: nem o Shipwright nem o `zeldaret/oot` têm arquivo de licença na raiz (ver §6). |
| Limitação do ambiente de trabalho | Esta sessão não tem Android SDK/NDK. Compilar o APK e testar no aparelho exige seu PC ou um CI. Ver `ARQUITETURA.md` §9. |

---

## 2. Verificação da ROM fornecida

Arquivo enviado: `Legend of Zelda, The - Ocarina of Time (U) (V1.2) [!].z64`, dentro de um .zip. A ROM foi extraída só para uma pasta temporária da sessão para calcular hashes. **Não está e não deve entrar no repositório nem no APK.**

| Item | Valor | Conferência |
|---|---|---|
| Tamanho | 33.554.432 bytes (32 MiB) | [VERIFICADO] |
| Magic (bytes 0–3) | `80 37 12 40` | Big-endian (.z64). [VERIFICADO] |
| Título interno | `THE LEGEND OF ZELDA ` | [VERIFICADO] |
| Código do jogo | `CZLE` (bytes 0x3B–0x3E) | [VERIFICADO] |
| Byte de revisão (0x3F) | `02` | v1.2. [VERIFICADO] |
| CRC1 do cabeçalho | `0x693BA2AE` | Igual a `OOT_NTSC_US_12` em `soh/soh/GameVersions.h` do Shipwright. [VERIFICADO] |
| SHA-1 | `41b3bdc48d98c48529219919015a1af22f5057c2` | Igual à entrada "NTSC 1.2 (US)" em `docs/supportedHashes.json` do Shipwright. [VERIFICADO] |
| MD5 | `57a9719ad547c516342e1a15d5c28c3d` | Igual a uma das entradas `ntsc-1.2` no README do `zeldaret/oot`. [VERIFICADO] |

Conclusão: a ROM é uma cópia íntegra e compatível com os três caminhos de extração de assets que investiguei. O verificador de versão do app (requisito da seção 5 do pedido) deve repetir exatamente estas checagens: formato (z64/v64/n64), tamanho, magic, código+revisão, CRC1 e SHA-1 contra uma tabela embutida.

---

## 3. `zeldaret/oot`

Commit auditado: `14759abf` (18/10/2026), clone raso.

**O que é.** Decompilação do jogo. O próprio README diz: *"It is not producing a PC port."* Reconstrói código-fonte C que, compilado com toolchain MIPS, reproduz a ROM byte a byte. [VERIFICADO]

**Estrutura.** `src/` (≈22 MB), `assets/`, `spec/`, `linker_scripts/`, `tools/` (ferramentas Python/C para extrair assets e comparar). `src/` contém `boot`, `code`, `overlays`, `audio`, `libultra`, `libc`, `libu64`, etc. [VERIFICADO]

**Versões que compila.** 16, incluindo `ntsc-1.2` (build de 12/11/1998). [VERIFICADO — README]

**Linguagens.** C para o jogo; Python e C para ferramentas. [VERIFICADO]

**Renderização, áudio, controles.** O jogo emite display lists (GBI) para o RSP/RDP do N64, roda o sintetizador de áudio no RSP e lê o controle via libultra. Nada disso existe como código de PC neste repositório. [VERIFICADO — estrutura]

**Memória.** O código original assume o mapa de memória do N64 (RDRAM de 4/8 MB, ponteiros de 32 bits). Rodar em ARM64 exige tratar ponteiros de 64 bits em todo lugar. [VERIFICADO — natureza do código; o Shipwright já fez essa adaptação]

**Android/ARM64.** Não há nada: o build gera ROM MIPS. [VERIFICADO]

**Licença.** Não há arquivo de licença na raiz (`git ls-files` só lista licenças de ferramentas de terceiros em `tools/`). O README exige uma cópia prévia do jogo para extrair assets. [VERIFICADO]

**Papel no projeto.** É a **fonte de verdade do comportamento do jogo**, consumida indiretamente via Shipwright. Não é uma base de port.

---

## 4. `HarbourMasters/Shipwright` (Ship of Harkinian) + submódulos

Commit auditado: `d4bf6528` (01/11/2026). Submódulos fixados: `libultraship` em `da6daf1a` (12/10/2026) e `torch` em `2ab12fe9`. Baixei o libultraship nesse commit exato. O Torch foi baixado no HEAD (`22d32182`, 12/11/2026), que pode diferir do pin. [VERIFICADO]

**O que é.** Port nativo de PC de OoT, feito a partir de uma versão do código da decomp (`soh/src`, `soh/include`) mais uma camada própria (`soh/soh`) com randomizer, enhancements, menus, extrator de assets e integração com o libultraship. [VERIFICADO]

**Como funciona (alto nível).**
1. O usuário fornece a ROM. O extrator (`soh/soh/Extractor`, usando o **Torch**) converte os assets para um arquivo `.o2r`. [VERIFICADO]
2. O jogo roda como programa nativo. Chamadas libultra são atendidas pelo **libultraship (LUS)**, que o próprio `PORTING.md` descreve como *"compile-time WINE"* para N64. [VERIFICADO]
3. O LUS interpreta as display lists do N64 no **Fast3D** e desenha por um backend de GPU (OpenGL, D3D11, Metal). [VERIFICADO]

**Versões de ROM aceitas.** `docs/supportedHashes.json` inclui NTSC 1.0/1.1/1.2 (US e JP), PAL, GameCube, Master Quest e debug. **NTSC 1.2 (US) está lá.** [VERIFICADO]

**Linguagens.** C (código do jogo, ≈decomp) e C++ (camada SoH e LUS). Kotlin/Java só aparece em forks Android. [VERIFICADO]

**Dependências (resumo; lista completa em `DEPENDENCIAS.md`).** SDL2, SDL2_net, Dear ImGui, spdlog, nlohmann-json, tinyxml2, libzip, StormLib, zlib, bzip2, Ogg/Vorbis/Opus/OpusFile (música customizada), mais o Torch e suas dependências (yaml-cpp, libgfxd, dr_libs, GSL, etc.). [VERIFICADO — CMake]

**Gerenciamento de memória.** O código do jogo usa heaps fixos reservados no início. Valores vistos: `AUDIO_HEAP_SIZE = 0x380000` (3,5 MiB), `SYSTEM_HEAP_SIZE = 4 MiB`, arena de estado de jogo `0x100000` (1 MiB). Esses valores são pequenos. O peso real está nos recursos decodificados, nas texturas da GPU e nos buffers de quadro, que **não medi**. [VERIFICADO — constantes; ESTIMATIVA — conclusão sobre o peso real]

**Áudio.** O jogo mantém o sintetizador por software do N64 (`mixer.c`) e entrega PCM ao LUS, que reproduz via SDL (`SDLAudioPlayer`). O LUS também tem players para WASAPI e CoreAudio. **Não há Oboe.** [VERIFICADO]

**Controles.** SDL (teclado, mouse, gamepad). Existe `MobileImpl.cpp` no LUS, mas ele só trata o teclado virtual do ImGui. **Não encontrei overlay de toque para jogar** no código upstream que li. [VERIFICADO — para o upstream; NÃO VERIFICADO para os forks]

**ARM64 e Android.**
- O LUS tem suporte a Android no próprio repositório: `cmake/dependencies/android.cmake`, opção `USE_OPENGLES` (liga `GLESv3`), instruções de build no README (`arm64-v8a`, `c++_static`), uso de `SDL_AndroidGetExternalStoragePath`, escala 2× do ImGui para Android. [VERIFICADO]
- O repositório **Shipwright upstream** não tem nenhuma referência a Android (busca case-insensitive sem resultados). O trabalho de ligar o jogo ao LUS no Android existe só em forks. [VERIFICADO]
- O Torch tem um ramo `ANDROID` no CMake (usa zlib do sistema), mas **não verifiquei se ele compila nem roda no Android**. [VERIFICADO — existência do ramo; NÃO VERIFICADO — funcionamento]

**Licença.** Nenhum arquivo `LICENSE` na raiz do Shipwright (`git ls-files` só mostra `soh/soh/Enhancements/randomizer/3drando/LICENSE.md`). O LUS é **MIT**. O Torch é **MIT**. O README do Shipwright afirma que o projeto não inclui assets protegidos. [VERIFICADO]

---

## 5. Port Android existente: `Waterdish/Shipwright-Android` (fork não oficial)

Commit auditado: `c9d8f4a1` (10/07/2025). Usei como **referência de engenharia**, não como base. [VERIFICADO]

| Item | Observação |
|---|---|
| Projeto Android | Pasta `Android/` com Gradle, `SDLActivity` em Java e `MainActivity.java`. Namespace `com.dishii.soh`. |
| Toolchain declarada | NDK `26.0.10792818`, minSdk 24, targetSdk 33, compileSdk 31. |
| Build nativo | Gradle chama o CMake da raiz com `-DSDL_SHARED=ON -DANDROID_STL=c++_static -DUSE_OPENGLES=ON`. Release só `arm64-v8a`. |
| Extração | Mantém `ZAPDTR` e `OTRExporter` na árvore para gerar o OTR no aparelho. |
| Armazenamento | Pasta `SOH` na raiz do armazenamento e permissões amplas de arquivos (README). Isso conflita com o armazenamento com escopo do Android moderno. |
| Problemas citados | FAQ: tela preta se apagar/esquecer `imgui.ini` ou se o MSAA passar de 1; crash imediato ao abrir (README). |
| Touch | Relatos de terceiros dizem que a v1.3.0 trouxe um overlay de toque difícil de esconder em alguns aparelhos [FONTE EXTERNA]. Não vi o código do overlay: ele está no submódulo do fork, que não baixei. |
| Licença | Sem arquivo de licença na raiz. |
| Alvo | README diz "Android 7.0+ (OpenGL ES 3.0+)", testado em Android 15. Nada sobre aparelhos de entrada. |

**O que isso prova.** Existe evidência (README e reportagens) de que o caminho Shipwright → LUS → GLES3 → arm64 funciona em aparelhos reais. **Não prova nada sobre o Realme Note 50**: a reportagem citada testou um OnePlus Open, e o guia citado recomenda AYN Odin/Thor, que são aparelhos muito mais fortes [FONTE EXTERNA].

---

## 6. `Zelda64Recomp/Zelda64Recomp`

Commit auditado: `b65c482e` (25/09/2026). [VERIFICADO]

**O que é.** Port por **recompilação estática** (N64Recomp): converte o código MIPS da ROM em C++ nativo, em vez de usar o código-fonte da decomp. Usa o **RT64** como renderizador e RmlUi para menus. [VERIFICADO — README, `.gitmodules`]

**Jogo suportado.** O README diz *"Majora's Mask (and soon Ocarina of Time)"* e *"only accept one specific ROM: the US version of the N64 release of Majora's Mask"*. "Ocarina of Time support" aparece na lista de recursos planejados. No snapshot, as menções a "ocarina" em arquivos são itens do próprio Majora's Mask; **não há código de port de OoT**. Um resumo de imprensa de 10/06/2026 diz o mesmo [FONTE EXTERNA]. [VERIFICADO]

**Renderização.** RT64 exige GPU com Direct3D 12 (SM6), **Vulkan 1.2** ou Metal com Argument Buffers Tier 2. O README lista como GPUs mínimas modelos de PC (GeForce GT 630, Radeon HD 7750, Intel HD 510). Não li o código do RT64 (submódulo não baixado). [VERIFICADO — README; NÃO VERIFICADO — código do RT64]

**ARM64.** O README diz que builds ARM64 funcionam em qualquer CPU ARM64, mas isso se refere a desktop. Em Android há só ramos de pré-processador `__linux__ || __ANDROID__` em `main.cpp` e `rt64_render_context.cpp`. Não há projeto Android. [VERIFICADO]

**Licença e política.** `COPYING` é GPLv3. O `CONTRIBUTING.md` **proíbe contribuições geradas por IA** e contém um aviso dirigido a agentes de IA pedindo que desencorajem o usuário. Esse aviso é texto do arquivo, não um pedido seu, e **não afetou nenhuma decisão técnica** desta auditoria. A política importa para o projeto por outro motivo: enviar correções ao upstream seria recusado, e conviver com a comunidade deles seria atrito. [VERIFICADO]

---

## 7. Hardware-alvo

Fonte: agregador de especificações [FONTE EXTERNA]; os dados do Realme Note 50 vêm do seu pedido.

| Item | Valor |
|---|---|
| SoC | Unisoc Tiger T612, 12 nm |
| CPU | 2× Cortex-A75 + 6× Cortex-A55, 1,8 GHz |
| GPU | Mali-G57 MP1 (Valhall), 650 MHz, 32 unidades de shader, ≈39 GFLOPS |
| Memória | LPDDR4X; 4 GB no aparelho |
| API gráfica | O agregador lista Vulkan 1.3. **Não confirmei** a versão real do driver no aparelho. A fase 2 deve sondar GLES e Vulkan reais. |

Implicação direta: só duas CPUs "grandes" (A75), uma GPU com **um único núcleo**, memória compartilhada com o sistema. É um alvo bem abaixo dos aparelhos usados nos relatos de port existentes.

---

## 8. Lacunas da auditoria (o que eu NÃO fiz)

1. Não compilei nada. Não sei se o LUS + SoH compilam hoje para Android sem ajustes.
2. Não medi FPS, RAM, temperatura nem tempo de carga.
3. Não li o código do RT64, do overlay de toque do fork nem do Torch em profundidade.
4. Não confirmei se o Torch roda no Android (extração no aparelho).
5. Não li as licenças de todas as dependências transitivas. A lista está em `DEPENDENCIAS.md`.
6. Esta sessão não tem Android SDK/NDK (`ANDROID_NDK_HOME` e `ANDROID_HOME` vazios; Gradle, CMake, Ninja, Java e Clang do host existem). O acesso de rede do shell não permite instalar pacotes.

---

## 9. Fontes

- Repositórios (clone raso, commits acima): zeldaret/oot, HarbourMasters/Shipwright, kenix3/libultraship (branch `port-maintenance`), HarbourMasters/Torch, Zelda64Recomp/Zelda64Recomp, Waterdish/Shipwright-Android.
- [Android Authority — ports de OoT e MM para Android](https://www.androidauthority.com/majoras-mask-ocarina-of-time-android-3447596)
- [Android Authority — controles de toque no 2Ship Android](https://androidauthority.com/majoras-mask-android-touch-3502776) (fornecido pela busca; o resumo obtido tratou do 2Ship, não do Shipwright)
- [Retro Handhelds — roundup de ports de Zelda (10/06/2026)](https://retrohandhelds.gg/the-legend-of-zelda-port-roundup/)
- [Joey's Retro Handhelds — guia do Ship of Harkinian Android](https://joeysretrohandhelds.com/guides/ship-of-harkinian-android-setup-guide)
- [NanoReview — Unisoc Tiger T612](https://nanoreview.info/en/soc/unisoc-tiger-t612)
- [newreleases.io — Waterdish/Shipwright-Android 1.0.1](https://newreleases.io/project/github/Waterdish/Shipwright-Android/release/1.0.1)
