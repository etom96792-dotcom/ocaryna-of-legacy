# ARQUITETURA.md — Fase 1 (proposta, aguardando aprovação)

Este documento responde aos itens 1–9 da "Primeira tarefa" do pedido. É uma **proposta**. Nada aqui foi implementado ou medido. Estimativas estão marcadas.

## 1. Melhor projeto-base

**Shipwright + libultraship + Torch.** Justificativa em `COMPARACAO_TECNICA.md`.

## 2. Arquitetura recomendada

```
┌────────────────────────────── APK (arm64-v8a) ───────────────────────────────┐
│ Kotlin (UI Android)                                                           │
│  ├─ Tela inicial / seletor de ROM (SAF) / verificador / configurações         │
│  ├─ Gerenciador de saves (listar, exportar, restaurar)                        │
│  └─ MainActivity : SDLActivity  ── lança o jogo                               │
│                                                                               │
│ JNI (fino)                                                                    │
│  ├─ verificarRom(fd) → {formato, versão, sha1, ok}                            │
│  ├─ extrair(fd, destino, callbackProgresso)                                   │
│  └─ ler/gravar configurações de toque, estado de ciclo de vida                │
│                                                                               │
│ C++ nativo                                                                    │
│  ├─ libsoh.so  = Shipwright (soh/src + soh/soh)                               │
│  ├─ libultraship (LUS) + Fast3D (backend GLES3)                               │
│  ├─ SDL2 (janela, GL, áudio, entrada)                                         │
│  └─ módulo novo: RomVerifier, SaveGuard, TouchOverlay (nativo ou ImGui)       │
└───────────────────────────────────────────────────────────────────────────────┘
Armazenamento privado do app: oot.o2r, saves, backups, configurações, cache de shaders
```

Princípios:
- **Camada Android fina, jogo intacto.** O jogo continua sendo o Shipwright. Mudanças no código do jogo só onde o Android exigir, e cada uma registrada no CHANGELOG.
- **Só OoT.** Sem seleção de console, biblioteca de ROMs ou outros núcleos.
- **Substituir apenas com causa comprovada.** Se algo falhar no Android, investigo a causa antes de trocar.

## 3. Dependências necessárias

Ver `DEPENDENCIAS.md`. Resumo: SDL2, ImGui, spdlog, nlohmann-json, tinyxml2, libzip, StormLib, zlib e o Torch na extração. Candidatas a remoção: SDL2_net e Ogg/Vorbis/Opus (a confirmar).

## 4. Dificuldades de portabilidade

| # | Dificuldade | Gravidade | Situação |
|---|---|---|---|
| 1 | O SoH upstream **não tem alvo Android** (nenhuma referência a "android" no repositório). O LUS tem. A ligação SoH ↔ LUS no Android existe só em forks | Alta | Trabalho novo, com fork como referência |
| 2 | **Torch no Android**: tem ramo `ANDROID` no CMake, mas ninguém confirmou compilação nem execução no aparelho | Alta | A verificar |
| 3 | **Armazenamento do Android 14**: o fork usa pasta na raiz e permissão ampla. Aqui será SAF + armazenamento privado | Média | Projeto definido (§6) |
| 4 | **Shaders em tempo de execução**: o Fast3D gera e compila shaders GLSL conforme o jogo pede. No `gfx_opengl.cpp` não achei cache de binário de programa (`glGetProgramBinary`). Em Mali isso tende a causar engasgos na primeira vez que cada combinação aparece | Alta | Hipótese a medir |
| 5 | **Precisão**: o backend GLES do LUS define `precision mediump float`. Em GPUs móveis, `mediump` pode causar artefatos de profundidade/coordenadas onde o desktop usa 32 bits | Média | Hipótese a testar |
| 6 | **Interface ImGui**: feita para mouse/teclado em desktop. O LUS já escala 2× no Android e trata o teclado virtual. O menu de jogo ainda precisa de redesenho para toque | Média | Trabalho novo |
| 7 | **Controles de toque**: não vi overlay de toque no upstream | Alta | Trabalho novo (Fase 5) |
| 8 | **Ciclo de vida do Android**: ir para segundo plano pode destruir o contexto EGL. O jogo e o cache de texturas da GPU precisam sobreviver a isso | Alta | A investigar na Fase 2/3 |
| 9 | **Áudio**: o sintetizador roda em software na CPU. Com 2 núcleos A75 e 6 A55, a thread de áudio pode competir com a de jogo | Média | A medir |
| 10 | **Memória na extração**: converter 32 MiB de ROM em `.o2r` consome RAM e CPU de forma pontual. Em 4 GB pode ser sensível | Média | A medir |
| 11 | **Térmica**: SoC de 12 nm, sem refrigeração ativa | Média | A medir na Fase 6 |
| 12 | **Licença do código** sem arquivo na raiz (SoH e decomp) | Alta (para publicar) | Decisão sua |
| 13 | **Projetos mudam rápido**: o SoH teve commit em 01/11/2026 e o LUS em 12/10/2026 | Média | Fixar commits (pins) e atualizar só de propósito |

## 5. Compatibilidade com ARM64

- O código do jogo no SoH já é 64 bits (o projeto roda em x86-64, e o README lista macOS, Linux, Switch e Wii U como plataformas). Não verifiquei um build ARM64 de desktop.
- O README do LUS já documenta build `arm64-v8a` com `c++_static`. [VERIFICADO]
- O fork Android compila release só para `arm64-v8a`. [VERIFICADO]
- Meta do app: **apenas `arm64-v8a`**, como o pedido manda. Sem armeabi-v7a nem x86.
- Ponto de atenção: alocação alinhada (`posix_memalign` com 0x10, já usada em `heaps.c`). Nenhum problema concreto de ARM64 encontrado na leitura; **só a compilação confirma**.

## 6. Estratégia de armazenamento (alinhada ao pedido)

| Dado | Onde | Observação |
|---|---|---|
| ROM do usuário | Escolhida via SAF. **Copiada para memória/arquivo temporário apenas durante a extração**, depois apagada | O app nunca guarda a ROM de forma permanente, a menos que o usuário peça |
| `oot.o2r` | Armazenamento privado do app | Dado derivado do jogo; nunca no repositório |
| Saves | Armazenamento privado + backup rotativo (3 gerações) + gravação atômica (arquivo temporário + rename) | Atende "proteção contra corrupção" e "recuperação após encerramento inesperado". **A confirmar** como o `SaveManager` do SoH grava hoje |
| Configurações e layout de toque | Armazenamento privado, JSON | — |
| Exportar/importar saves | SAF | Opcional, Fase 7 |

## 7. Estratégia de renderização

Ordem, respeitando "não implemente melhorias gráficas antes de o original funcionar":

1. **Fase 4:** o Fast3D com backend **GLES3** do LUS, como está. Objetivo: imagem correta e estável. Resolução interna do original (4:3, escalável).
2. **Medir**: tempo de quadro, número de draw calls, tempo de compilação de shader, uso de GPU.
3. **Depois, só com medição:**
   - cache de shaders (binário de programa ou pré-aquecimento das combinações conhecidas);
   - escala de renderização;
   - filtro de textura (nearest / bilinear / 3-point);
   - limite de FPS e V-Sync;
   - modo de economia de energia (limite de FPS + escala menor).
4. **Sobre FPS:** a lógica original do jogo roda a 20 Hz. O código do SoH tem `frame_interpolation.cpp`. Verificar na Fase 4 se interpolar 20 → 30 é viável e estável neste hardware. **Não afirmo que 30 FPS estáveis são alcançáveis**; é a meta, a ser comprovada.
5. **Vulkan:** adiado. Só se o profiling mostrar o GLES como gargalo e houver ganho esperado.

## 8. Estimativa de consumo de memória

**Isto é uma estimativa de baixa confiança. Não há medição.**

| Componente | Valor | Origem |
|---|---|---|
| Heap do sistema do jogo | 4 MiB | `SYSTEM_HEAP_SIZE`, verificado |
| Heap de áudio | 3,5 MiB | `AUDIO_HEAP_SIZE = 0x380000`, verificado |
| Arena de estado de jogo | 1 MiB | `GameState_InitArena(…, 0x100000)`, verificado |
| Recursos decodificados (texturas, modelos, áudio) e `.o2r` | **desconhecido** | O jogo faz pré-carga do diretório `audio` e cache de recursos |
| Texturas e buffers na GPU | **desconhecido** | Memória compartilhada com a RAM em SoC móvel |
| SDL, ImGui, libs, runtime Android | **desconhecido** | — |

Hipótese de trabalho: **algumas centenas de MB adicionais**, plausivelmente dentro de 500 MB. Isso é palpite; a meta de "até 500 MB" só será confirmada com `dumpsys meminfo` no aparelho. Com 4 GB físicos e o sistema Android 14 ocupando boa parte, o teto real para o app pode ser bem menor que 4 GB; o valor exato **precisa ser medido** no aparelho.

## 9. Plano de compilação Android

**Restrição do ambiente:** esta sessão não tem Android SDK/NDK e não consegue instalar pacotes. Isso afeta o "Compile" e o "Teste" da metodologia. Opções:

| Opção | Como | Vantagem | Desvantagem |
|---|---|---|---|
| A (recomendada) | Compilo o projeto aqui só até onde o ambiente permite (código novo, scripts, CMake de configuração). O APK é compilado por **GitHub Actions** (workflow que eu escrevo) ou no seu PC | APK reproduzível; não depende da sua máquina | Precisa de repositório no GitHub com Actions; compilação do SoH é pesada |
| B | Você compila no PC com Android Studio + NDK, seguindo `BUILD_INSTRUCTIONS.md` | Controle total | Mais trabalho manual para você |
| C | Liberar a rede/instalação de pacotes nesta sessão (configuração da conta) para eu baixar o NDK | Eu compilo e itero aqui | Depende de configuração que não controlo; um chat novo pode ser necessário |

**Testes no aparelho** não são possíveis aqui em nenhuma opção. Para FPS, RAM, temperatura e estabilidade, você precisará rodar o APK no Realme Note 50 e me enviar capturas de `logcat`, `dumpsys meminfo` e `dumpsys gfxinfo`. Vou preparar um roteiro objetivo para isso.

Passos de build planejados (Fase 2):
1. Projeto Gradle (Kotlin) com módulo nativo via CMake externo.
2. `ANDROID_ABI=arm64-v8a`, `ANDROID_STL=c++_static`, `USE_OPENGLES=ON`, `SDL_SHARED=ON` (como o fork).
3. NDK e `minSdk`: decididos na Fase 2 após testar (fork usa NDK 26 e minSdk 24 como referência).
4. Commits do LUS e do Torch fixados nos pins do Shipwright, para builds reprodutíveis.
5. Pipeline: `./gradlew assembleRelease` → `app-release.apk` (assinado com chave de debug no início).

## 10. Primeiro protótipo técnico a construir

Objetivo: **responder as perguntas de maior risco com o menor código possível.** Não é o app final, e **não simula o jogo**.

**Protótipo 1 — "sonda + extração" (Fase 2, depois da sua aprovação)**

1. **Sonda de hardware** (APK mínimo, Kotlin + SDL2):
   - versão real de GLES e `GL_RENDERER` do Mali-G57;
   - extensões disponíveis; Vulkan presente ou não;
   - RAM total/disponível; tamanho do heap do app;
   - resultado gravado em arquivo e mostrado na tela.
2. **Verificador de ROM em C++** (JNI) com os testes da §2 de `AUDITORIA.md`, testado com a ROM fornecida em um teste de host (Linux), sem Android.
3. **Build do LUS + SoH para arm64** (via NDK) até gerar `libsoh.so`, registrando cada erro de compilação e sua causa.
4. **Extração**: tentar gerar `oot.o2r` com o Torch (no host primeiro, para saber o tamanho e o tempo; no aparelho depois).

Critério de sucesso: **APK que abre, mostra os dados da sonda e verifica a ROM**, mais um relatório de compilação do SoH para arm64 com a lista exata de bloqueios. Só depois disso a Fase 3 (primeira execução real de OoT) começa.

## 11. Perguntas que preciso que você responda para a Fase 2

Estão no relatório final da conversa e em `MASTER_CONTEXT.md`:
1. Como compilar o APK (opção A, B ou C da §9).
2. Quem testa no aparelho e como me devolve os dados.
3. Se há intenção de publicar o APK (impacta a questão de licença).
4. Se posso tornar opcionais SDL2_net e a música customizada para reduzir o tamanho.
