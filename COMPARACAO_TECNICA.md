# COMPARACAO_TECNICA.md — Fase 1

Comparação das três alternativas pedidas, mais o fork Android como referência. Detalhes e níveis de confiança estão em `AUDITORIA.md`.

## 1. Tabela comparativa

| Critério | `zeldaret/oot` | Shipwright + libultraship | Zelda64Recomp |
|---|---|---|---|
| Natureza | Decompilação (gera a ROM) | Port nativo a partir da decomp | Port por recompilação estática |
| É um port jogável? | Não | Sim (PC, Linux, macOS, Switch, Wii U) | Só Majora's Mask hoje |
| Ocarina of Time | Sim (fonte da verdade) | Sim | **Não** ("em breve"; sem código de OoT no snapshot) |
| ROM NTSC-U v1.2 | Compila `ntsc-1.2` | Aceita (SHA-1 conferido) | Aceita só o MM US |
| Linguagem | C + ferramentas | C (jogo) + C++ (LUS, camada SoH) | C++ (runtime) + código MIPS recompilado |
| Vídeo | Nenhum | Fast3D (OpenGL, D3D11, Metal); **GLES3 via `USE_OPENGLES`** | RT64: D3D12 / **Vulkan 1.2** / Metal |
| Áudio | Nenhum | Sintetizador do jogo → SDL (`SDLAudioPlayer`) | Código próprio do projeto (não auditado) |
| Controles | Nenhum | SDL (teclado, mouse, gamepad); sem overlay de toque no upstream | Controle e teclado (README); sem toque |
| Android no repositório | Nenhum | **LUS: sim** (CMake, GLES, SDL). **SoH upstream: não** | Só `#ifdef __ANDROID__` em 2 arquivos |
| Port Android existente | Não | Sim, fork não oficial (Waterdish) | Não encontrei |
| Peso gráfico | — | Moderado: interpretador de display lists + GLES3 | Alto para o alvo: exige recursos Vulkan 1.2 de PC |
| Licença do código | Sem arquivo de licença | SoH: sem arquivo de licença na raiz. LUS e Torch: MIT | GPLv3 |
| Política de IA | Não vi | Não vi | **Proíbe contribuições por IA** (CONTRIBUTING.md) |
| Manutenção recente | Commit 18/10/2026 | Commit 01/11/2026; LUS 12/10/2026; Torch 12/11/2026 | Commit 25/09/2026 |

## 2. Pontuação por critério do pedido

Escala 0–5, julgamento meu a partir do que li. Não é medição.

| Critério | oot | Shipwright+LUS | Zelda64Recomp |
|---|---|---|---|
| Executa OoT hoje | 0 | 5 | 0 |
| Esforço para chegar a algo jogável no Android | 0 | 4 | 1 |
| Compatibilidade com GPU Mali-G57 MP1 | — | 3 | 1 |
| Footprint de memória provável | — | 3 | 2 |
| Clareza de licença | 1 | 2 | 4 (GPLv3) |
| Possibilidade de ter UI própria, só OoT | 1 | 4 | 3 |
| Risco de depender de terceiros | 3 | 3 | 5 |

Notas:
- "Compatibilidade com Mali-G57" no Shipwright é 3 e não 5 porque o caminho GLES3 existe, mas **ninguém mediu** nesse hardware.
- Zelda64Recomp perde por duas razões independentes: não tem OoT e o RT64 pede mais da GPU do que o alvo provavelmente entrega.

## 3. Decisão

**Base: Shipwright (SoH) + libultraship, com Torch para extração.**

Motivos, em ordem de peso:
1. É o único dos três que **já executa Ocarina of Time** como programa nativo.
2. O libultraship já traz o que o pedido manda investigar para Android: backend GLES3, SDL2, CMake para NDK arm64.
3. O jogo em si (`soh/src`) já foi adaptado a ponteiros de 64 bits. Refazer isso a partir do `zeldaret/oot` seria reescrever o Shipwright.
4. Há forks Android como evidência de viabilidade e como fonte de pontos de integração (Gradle + `SDLActivity`).

**O que isso NÃO significa:**
- Não significa copiar o fork Android. O fork usa armazenamento fora do modelo moderno do Android e mantém ferramentas de extração antigas. A recomendação é um shell Android novo, só para OoT.
- Não significa que o desempenho no Realme Note 50 está garantido. Isso é a primeira coisa a medir.

## 4. Alternativas descartadas e quando reconsiderar

| Alternativa | Reconsiderar se |
|---|---|
| Zelda64Recomp | Lançar suporte a OoT **e** o RT64 ganhar um backend adequado a GPUs móveis de entrada. Hoje nenhuma das duas condições vale. |
| `zeldaret/oot` direto | Só se o objetivo virar "port do zero". Custo muito acima do pedido. |
| Emulador N64 genérico | Fora do pedido (seção 5 do prompt). |

## 5. Impacto de licença na escolha

Não sou advogado e isto não é parecer jurídico. Fatos observados:
- LUS e Torch são MIT.
- O Shipwright e o `zeldaret/oot` não têm arquivo de licença na raiz. Isso significa que **não está declarado, no repositório, em que termos o código pode ser redistribuído**.
- O Zelda64Recomp é GPLv3, o que implica obrigações de redistribuir código-fonte de derivados.

Consequência prática: **antes de publicar qualquer APK**, vale pedir esclarecimento aos mantenedores ou consultar alguém habilitado. Para uso pessoal e distribuição só do código-fonte do app (sem assets), o risco é menor, mas a decisão é sua.
